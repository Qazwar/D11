#include "ResourceContainer.h"
#include <D3D11.h>
#include <D3Dcompiler.h>
#include <map>
#include "..\io\json.h"
#include "ResourceDescriptors.h"
#include "..\renderer\graphics.h"
#include <string.h>
#include "..\utils\Log.h"
#include "..\renderer\render_types.h"
#include "..\imgui\IMGUI.h"
#include "..\utils\Assert.h"
//#include "..\utils\ObjLoader.h"
//#include "..\renderer\RenderTarget.h"
//#include "..\renderer\SkyBox.h"
#include "Resource.h"

namespace ds {

	namespace res {	

		typedef void(*ParseFunc)(JSONReader&, int);

		// ------------------------------------------------------
		// resource index
		// ------------------------------------------------------
		struct ResourceIndex {
			RID id;
			ResourceType type;
			uint32_t nameIndex;
			IdString hash;
		};

		// ------------------------------------------------------
		// resource context
		// ------------------------------------------------------
		struct ResourceContext {

			CharBuffer nameBuffer;
			ID3D11Device* device;
			uint32_t resourceIndex;
			Array<BaseResource*> resources;
			Array<ResourceIndex> indices;
			std::map<IdString, ParseFunc> parsers;
			ParticleManager* particles;
			
		};

		static ResourceContext* _resCtx;

		struct BlendStateMapping {
			const char* name;
			D3D11_BLEND blend;
			BlendStateMapping(const char* n, D3D11_BLEND b) : name(n), blend(b) {}
		};

		static const BlendStateMapping BLEND_STATE_MAPPINGS[] = {
			{ "ZERO", D3D11_BLEND_ZERO },
			{ "ONE", D3D11_BLEND_ONE },
			{ "SRC_COLOR", D3D11_BLEND_SRC_COLOR },
			{ "INV_SRC_COLOR", D3D11_BLEND_INV_SRC_COLOR },
			{ "SRC_ALPHA", D3D11_BLEND_SRC_ALPHA },
			{ "INV_SRC_ALPHA", D3D11_BLEND_INV_SRC_ALPHA },
			{ "DEST_ALPHA", D3D11_BLEND_DEST_ALPHA },
			{ "INV_DEST_ALPHA", D3D11_BLEND_INV_DEST_ALPHA },
			{ "DEST_COLOR", D3D11_BLEND_DEST_COLOR },
			{ "INV_DEST_COLOR", D3D11_BLEND_INV_DEST_COLOR },
			{ "SRC_ALPHA_SAT", D3D11_BLEND_SRC_ALPHA_SAT },
			{ "BLEND_FACTOR", D3D11_BLEND_BLEND_FACTOR },
			{ "INV_BLEND_FACTOR", D3D11_BLEND_INV_BLEND_FACTOR },
			{ "SRC1_COLOR", D3D11_BLEND_SRC1_COLOR },
			{ "INV_SRC1_COLOR", D3D11_BLEND_INV_SRC1_COLOR },
			{ "SRC1_ALPHA", D3D11_BLEND_SRC1_ALPHA },
			{ "INV_SRC1_ALPHA", D3D11_BLEND_INV_SRC1_ALPHA },
		};

		struct InputElementDescriptor {

			const char* semantic;
			DXGI_FORMAT format;
			uint32_t size;

			InputElementDescriptor(const char* sem, DXGI_FORMAT f, uint32_t s) :
				semantic(sem), format(f), size(s) {
			}
		};

		static const InputElementDescriptor INPUT_ELEMENT_DESCRIPTIONS[] = {

			{ "POSITION", DXGI_FORMAT_R32G32B32_FLOAT, 12 },
			{ "COLOR",    DXGI_FORMAT_R32G32B32A32_FLOAT, 16 },
			{ "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT, 8 },
			{ "NORMAL",   DXGI_FORMAT_R32G32B32_FLOAT, 12 }
		};

		struct TextureAddressModeMapping {
			const char* name;
			D3D11_TEXTURE_ADDRESS_MODE mode;
		};

		static const TextureAddressModeMapping TEXTURE_ADDRESS_MODES[] = {
			{ "WRAP", D3D11_TEXTURE_ADDRESS_WRAP },
			{ "MIRROR", D3D11_TEXTURE_ADDRESS_MIRROR },
			{ "CLAMP", D3D11_TEXTURE_ADDRESS_CLAMP },
			{ "BORDER", D3D11_TEXTURE_ADDRESS_BORDER },
			{ "MIRROR_ONCE", D3D11_TEXTURE_ADDRESS_MIRROR_ONCE },
		};

		static const char* ResourceTypeNames[] = {
			"SHADER",
			"TEXTURE",
			"VERTEXBUFFER",
			"INDEXBUFFER",
			"CONSTANTBUFFER",
			"BLENDSTATE",
			"INPUTLAYOUT",
			"BITMAPFONT",
			"SPRITEBUFFER",
			"WORLD",
			"SAMPLERSTATE",
			"GUIDIALOG",
			"QUADBUFFER",
			"MESHBUFFER",
			"MESH",
			"SCENE",
			"CAMERA",
			"RENDERTARGET",
			"SKYBOX",
			"MATERIAL",
			"PARTICLEMANAGER",
			"UNKNOWN"
		};

		

		// ------------------------------------------------------
		// find sampler state by name
		// ------------------------------------------------------
		static int findTextureAddressMode(const char* name) {
			for (int i = 0; i < 5; ++i) {
				if (strcmp(TEXTURE_ADDRESS_MODES[i].name, name) == 0) {
					return i;
				}
			}
			return -1;
		}

		// ------------------------------------------------------
		// find blendstate by name
		// ------------------------------------------------------
		int findBlendState(const char* text) {
			for (int i = 0; i < 17; ++i) {
				if (strcmp(BLEND_STATE_MAPPINGS[i].name, text) == 0) {
					return i;
				}
			}
			return -1;
		}

		// ------------------------------------------------------
		// find inputelement by name
		// ------------------------------------------------------
		static int findInputElement(const char* name) {
			for (int i = 0; i < 7; ++i) {
				if (strcmp(INPUT_ELEMENT_DESCRIPTIONS[i].semantic, name) == 0) {
					return i;
				}
			}
			return -1;
		}
		
		// ------------------------------------------------------
		// shutdown
		// ------------------------------------------------------
		void shutdown() {
			// delete all resources
			for (size_t i = 0; i < _resCtx->resources.size(); ++i) {
				delete _resCtx->resources[i];
			}

			gui::shutdown();
			delete _resCtx;
		}

		// ------------------------------------------------------
		// cretate internal resource entry
		// ------------------------------------------------------
		static RID create(const char* name, ResourceType type) {
			IdString hash = string::murmur_hash(name);
			ResourceIndex ri;
			// this one gets called after we have already the resource
			ri.id = _resCtx->resources.size() - 1;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = type;
			ri.hash = hash;
			LOG << "adding resource: '" << name << "' at " << ri.id << " type: " << ResourceTypeNames[type];
			_resCtx->indices.push_back(ri);
			return ri.id;
		}

		// ------------------------------------------------------
		// cretate quad index buffer
		// ------------------------------------------------------
		static RID createQuadIndexBuffer(const char* name,const QuadIndexBufferDescriptor& descriptor) {
			
			// FIXME: check that size % 6 == 0 !!!
			D3D11_BUFFER_DESC bufferDesc;
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.CPUAccessFlags = 0;
			bufferDesc.ByteWidth = sizeof(uint32_t) * descriptor.size;
			bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bufferDesc.MiscFlags = 0;

			uint32_t* data = new uint32_t[descriptor.size];
			int base = 0;
			int cnt = 0;
			int num = descriptor.size / 6;
			for (int i = 0; i < num; ++i) {
				data[base] = cnt;
				data[base + 1] = cnt + 1;
				data[base + 2] = cnt + 3;
				data[base + 3] = cnt + 1;
				data[base + 4] = cnt + 2;
				data[base + 5] = cnt + 3;
				base += 6;
				cnt += 4;
			}
			// Define the resource data.
			D3D11_SUBRESOURCE_DATA InitData;
			InitData.pSysMem = data;
			InitData.SysMemPitch = 0;
			InitData.SysMemSlicePitch = 0;
			// Create the buffer with the device.
			ID3D11Buffer* buffer;
			HRESULT hr = _resCtx->device->CreateBuffer(&bufferDesc, &InitData, &buffer);
			if (FAILED(hr)) {
				delete[] data;
				DXTRACE_MSG("Failed to create quad index buffer!");
				return -1;
			}
			delete[] data;
			RID rid = create(name, ResourceType::INDEXBUFFER);
			IndexBufferResource* cbr = new IndexBufferResource(buffer);
			_resCtx->resources.push_back(cbr);
			return rid;
		}

		// ------------------------------------------------------
		// create index buffer
		// ------------------------------------------------------
		static RID createIndexBuffer(const char* name, const IndexBufferDescriptor& descriptor) {
			D3D11_BUFFER_DESC bufferDesc;
			if (descriptor.dynamic) {
				bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
				bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			}
			else {
				bufferDesc.Usage = D3D11_USAGE_DEFAULT;
				bufferDesc.CPUAccessFlags = 0;
			}
			bufferDesc.ByteWidth = sizeof(uint32_t) * descriptor.size;
			bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bufferDesc.MiscFlags = 0;
			ID3D11Buffer* buffer;
			HRESULT hr = _resCtx->device->CreateBuffer(&bufferDesc, 0, &buffer);
			if (FAILED(hr)) {
				DXTRACE_MSG("Failed to create index buffer!");
				return -1;
			}
			IndexBufferResource* cbr = new IndexBufferResource(buffer);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::INDEXBUFFER);
		}

		// ------------------------------------------------------
		// create constant buffer
		// ------------------------------------------------------
		RID createConstantBuffer(const char* name, const ConstantBufferDescriptor& descriptor) {
			D3D11_BUFFER_DESC constDesc;
			ZeroMemory(&constDesc, sizeof(constDesc));
			constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			constDesc.ByteWidth = descriptor.size;
			constDesc.Usage = D3D11_USAGE_DYNAMIC;
			constDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			ID3D11Buffer* buffer = 0;
			HRESULT d3dResult = _resCtx->device->CreateBuffer(&constDesc, 0, &buffer);
			if (FAILED(d3dResult))	{
				DXTRACE_MSG("Failed to create constant buffer!");
				return -1;
			}
			ConstantBufferResource* cbr = new ConstantBufferResource(buffer);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::CONSTANTBUFFER);
		}

		// ------------------------------------------------------
		// create material
		// ------------------------------------------------------
		RID createMaterial(const char* name, const MaterialDescriptor& descriptor) {
			Material* m = new Material;
			m->ambient = descriptor.ambient;
			m->diffuse = descriptor.diffuse;
			m->blendState = descriptor.blendstate;
			m->shader = descriptor.shader;
			m->texture = descriptor.texture;
			m->renderTarget = descriptor.renderTarget;
			MaterialResource* cbr = new MaterialResource(m);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::MATERIAL);
		}

		// ------------------------------------------------------
		// create render target
		// ------------------------------------------------------
		static RID createRenderTarget(const char* name, const RenderTargetDescriptor& descriptor) {
			RenderTarget* rt = new RenderTarget(descriptor);
			bool ret = rt->init(graphics::getDevice(), descriptor.width, descriptor.height);
			XASSERT(ret, "Could not create rendertarget: %s", name);
			RenderTargetResource* cbr = new RenderTargetResource(rt);			
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::RENDERTARGET);
		}

		// ------------------------------------------------------
		// create sprite buffer
		// ------------------------------------------------------
		static RID createSpriteBuffer(const char* name, const SpriteBufferDescriptor& descriptor) {
			SpriteBuffer* buffer = new SpriteBuffer(descriptor);
			SpriteBufferResource* cbr = new SpriteBufferResource(buffer);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::SPRITEBUFFER);
		}

		// ------------------------------------------------------
		// create quad buffer
		// ------------------------------------------------------
		static RID createQuadBuffer(const char* name, const QuadBufferDescriptor& descriptor) {
			QuadBuffer* buffer = new QuadBuffer(descriptor);
			QuadBufferResource* cbr = new QuadBufferResource(buffer);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::QUADBUFFER);
		}

		// ------------------------------------------------------
		// create mesh buffer
		// ------------------------------------------------------
		static RID createMeshBuffer(const char* name, const MeshBufferDescriptor& descriptor) {
			MeshBuffer* buffer = new MeshBuffer(descriptor);
			MeshBufferResource* cbr = new MeshBufferResource(buffer);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::MESHBUFFER);
		}

		

		// ------------------------------------------------------
		// create skybox
		// ------------------------------------------------------
		static RID createSkyBox(const char* name, const SkyBoxDescriptor& descriptor) {
			SkyBox* buffer = new SkyBox(descriptor);
			SkyBoxResource* cbr = new SkyBoxResource(buffer);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::SKYBOX);
		}

		// ------------------------------------------------------
		// create mesh 
		// ------------------------------------------------------
		static RID createMesh(const char* name, const MeshDescriptor& descriptor) {
			Mesh* mesh = new Mesh;
			mesh->load(descriptor.fileName);
			MeshResource* cbr = new MeshResource(mesh);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::MESH);
		}

		

		// ------------------------------------------------------
		// create sampler state
		// ------------------------------------------------------
		RID createSamplerState(const char* name, const SamplerStateDescriptor& descriptor) {
			D3D11_SAMPLER_DESC colorMapDesc;
			ZeroMemory(&colorMapDesc, sizeof(colorMapDesc));
			colorMapDesc.AddressU = TEXTURE_ADDRESS_MODES[descriptor.addressU].mode;
			colorMapDesc.AddressV = TEXTURE_ADDRESS_MODES[descriptor.addressV].mode;
			colorMapDesc.AddressW = TEXTURE_ADDRESS_MODES[descriptor.addressW].mode;
			colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			//colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			//colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			colorMapDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;
			ID3D11SamplerState* sampler;
			HRESULT d3dResult = _resCtx->device->CreateSamplerState(&colorMapDesc, &sampler);
			if (FAILED(d3dResult)) {
				DXTRACE_MSG("Failed to create SamplerState!");
				return INVALID_RID;
			}
			SamplerStateResource* cbr = new SamplerStateResource(sampler);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::SAMPLERSTATE);
		}

		// ------------------------------------------------------
		// load texture
		// ------------------------------------------------------
		static RID loadTexture(const char* name, const TextureDescriptor& descriptor) {
			ID3D11ShaderResourceView* srv = 0;
			char buffer[256];
			sprintf_s(buffer, 256, "content\\textures\\%s", descriptor.name);
			HRESULT d3dResult = D3DX11CreateShaderResourceViewFromFile(_resCtx->device, buffer, 0, 0, &srv, 0);
			if (FAILED(d3dResult)) {
				DXTRACE_MSG("Failed to load the texture image!");
				return INVALID_RID;
			}
			ShaderResourceViewResource* cbr = new ShaderResourceViewResource(srv);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::TEXTURE);
		}

		// ------------------------------------------------------
		// load texture cube
		// ------------------------------------------------------
		static RID loadTextureCube(const char* name, const TextureDescriptor& descriptor) {
			ID3D11ShaderResourceView* srv = 0;
			char buffer[256];
			sprintf_s(buffer, 256, "content\\textures\\%s", descriptor.name);

			D3DX11_IMAGE_LOAD_INFO loadSMInfo;
			loadSMInfo.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

			//Load the texture
			ID3D11Texture2D* SMTexture = 0;
			HRESULT hr = D3DX11CreateTextureFromFile(_resCtx->device, buffer, &loadSMInfo, 0, (ID3D11Resource**)&SMTexture, 0);

			//Create the textures description
			D3D11_TEXTURE2D_DESC SMTextureDesc;
			SMTexture->GetDesc(&SMTextureDesc);

			//Tell D3D We have a cube texture, which is an array of 2D textures
			D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc;
			SMViewDesc.Format = SMTextureDesc.Format;
			SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			SMViewDesc.TextureCube.MipLevels = SMTextureDesc.MipLevels;
			SMViewDesc.TextureCube.MostDetailedMip = 0;

			//Create the Resource view
			hr = _resCtx->device->CreateShaderResourceView(SMTexture, &SMViewDesc, &srv);
			
			if (FAILED(hr)) {
				DXTRACE_MSG("Failed to load the texture image!");
				return INVALID_RID;
			}
			ShaderResourceViewResource* cbr = new ShaderResourceViewResource(srv);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::TEXTURE);
		}

		// ------------------------------------------------------
		// load bitmap font
		// ------------------------------------------------------
		static RID loadFont(const char* name,const BitmapfontDescriptor& descriptor) {
			char buffer[256];
			sprintf_s(buffer, 256, "content\\resources\\%s", descriptor.name);
			float xOffset = 0.0f;
			float yOffset = 0.0f;
			float textureSize = 1024.0f;
			Bitmapfont* font = new Bitmapfont;
			JSONReader reader;
			bool ret = reader.parse(buffer);
			assert(ret);
			int info = reader.find_category("settings");
			if (info != -1) {
				reader.get_float(info, "x_offset", &xOffset);
				reader.get_float(info, "y_offset", &yOffset);
				reader.get_float(info, "texture_size", &textureSize);
			}
			int num = reader.find_category("characters");
			char tmp[16];
			Rect rect;
			if (num != -1) {
				for (int i = 32; i < 255; ++i) {
					sprintf_s(tmp, 16, "C%d", i);
					if (reader.contains_property(num, tmp)) {
						reader.get(num, tmp, &rect);
						font->add(i, rect, xOffset, yOffset, textureSize);
					}
				}
			}
			BitmapfontResource* cbr = new BitmapfontResource(font);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::BITMAPFONT);
		}

		// ------------------------------------------------------
		// create blend state
		// ------------------------------------------------------
		RID createBlendState(const char* name, const BlendStateDescriptor& descriptor) {
			D3D11_BLEND_DESC blendDesc;
			ZeroMemory(&blendDesc, sizeof(blendDesc));
			if (descriptor.alphaEnabled) {
				blendDesc.RenderTarget[0].BlendEnable = TRUE;
			}
			else {
				blendDesc.RenderTarget[0].BlendEnable = FALSE;
				//blendDesc.RenderTarget[0].BlendEnable = (srcBlend != D3D11_BLEND_ONE) || (destBlend != D3D11_BLEND_ZERO);
			}
			blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlend = BLEND_STATE_MAPPINGS[descriptor.srcBlend].blend;
			blendDesc.RenderTarget[0].DestBlend = BLEND_STATE_MAPPINGS[descriptor.destBlend].blend;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlendAlpha = BLEND_STATE_MAPPINGS[descriptor.srcAlphaBlend].blend;
			blendDesc.RenderTarget[0].DestBlendAlpha = BLEND_STATE_MAPPINGS[descriptor.destAlphaBlend].blend;
			blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;

			ID3D11BlendState* state;
			HRESULT d3dResult = _resCtx->device->CreateBlendState(&blendDesc, &state);
			if (FAILED(d3dResult)) {
				DXTRACE_MSG("Failed to create blendstate!");
				return INVALID_RID;
			}
			BlendStateResource* cbr = new BlendStateResource(state);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::BLENDSTATE);
		}

		// ------------------------------------------------------
		// create scene
		// ------------------------------------------------------
		static RID createScene(const char* name, const SceneDescriptor& descriptor) {
			Scene* scene = new Scene(descriptor);
			int idx = _resCtx->resources.size();
			SceneResource* cbr = new SceneResource(scene);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::SCENE);
		}

		// ------------------------------------------------------
		// create dialog
		// ------------------------------------------------------
		static RID createDialog(const char* name, const GUIDialogDescriptor& descriptor) {
			GUIDialog* dialog = new GUIDialog(descriptor);
			dialog->load();
			int idx = _resCtx->resources.size();
			GUIDialogResource* cbr = new GUIDialogResource(dialog);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::GUIDIALOG);
		}

		// ------------------------------------------------------
		// create particle manager
		// ------------------------------------------------------
		static RID createParticleManager(const ParticleSystemsDescriptor& descriptor) {
			_resCtx->particles = new ParticleManager(descriptor);
			_resCtx->particles->load();
			int idx = _resCtx->resources.size();
			ParticleManagerResource* cbr = new ParticleManagerResource(_resCtx->particles);
			_resCtx->resources.push_back(cbr);
			return create("ParticleManager", ResourceType::PARTICLEMANAGER);
		}

		// ------------------------------------------------------
		// create vertex buffer
		// ------------------------------------------------------
		RID createVertexBuffer(const char* name, const VertexBufferDescriptor& descriptor) {
			InputLayoutResource* res = static_cast<InputLayoutResource*>(_resCtx->resources[descriptor.layout]);
			UINT size = descriptor.size * res->size();

			D3D11_BUFFER_DESC bufferDesciption;
			ZeroMemory(&bufferDesciption, sizeof(bufferDesciption));
			if (descriptor.dynamic) {
				bufferDesciption.Usage = D3D11_USAGE_DYNAMIC;
				bufferDesciption.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			}
			else {
				bufferDesciption.Usage = D3D11_USAGE_DEFAULT;
				bufferDesciption.CPUAccessFlags = 0;
			}
			bufferDesciption.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDesciption.ByteWidth = size;

			ID3D11Buffer* buffer = 0;
			HRESULT d3dResult = _resCtx->device->CreateBuffer(&bufferDesciption, 0, &buffer);
			if (FAILED(d3dResult))	{
				DXTRACE_MSG("Failed to create buffer!");
				return -1;
			}
			int idx = _resCtx->resources.size();
			VertexBufferResource* cbr = new VertexBufferResource(buffer, size, descriptor.layout);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::VERTEXBUFFER);
		}

		// ------------------------------------------------------
		// create vertex buffer
		// ------------------------------------------------------
		RID createInputLayout(const char* name, const InputLayoutDescriptor& descriptor) {
			D3D11_INPUT_ELEMENT_DESC* descriptors = new D3D11_INPUT_ELEMENT_DESC[descriptor.num];
			uint32_t index = 0;
			uint32_t counter = 0;			
			int si[8] = { 0 };
			for (int i = 0; i < descriptor.num; ++i) {
				const InputElementDescriptor& d = INPUT_ELEMENT_DESCRIPTIONS[descriptor.indices[i]];
				D3D11_INPUT_ELEMENT_DESC& desc = descriptors[i];
				desc.SemanticName = d.semantic;
				desc.SemanticIndex = si[descriptor.indices[i]];// d.semanticIndex;
				desc.Format = d.format;
				desc.InputSlot = 0;
				desc.AlignedByteOffset = index;
				desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				desc.InstanceDataStepRate = 0;
				index += d.size;
				si[descriptor.indices[i]] += 1;
			}			
			ID3D11InputLayout* layout = 0;
			if (descriptor.shader == INVALID_RID) {
				_resCtx->device->CreateInputLayout(descriptors, descriptor.num, descriptor.byteCode, descriptor.byteCodeSize, &layout);
			}
			else {
				ShaderResource* sr = static_cast<ShaderResource*>(_resCtx->resources[descriptor.shader]);
				Shader* s = sr->get();
				assert(s != 0);
				HRESULT d3dResult = _resCtx->device->CreateInputLayout(descriptors, descriptor.num, s->vertexShaderBuffer->GetBufferPointer(), s->vertexShaderBuffer->GetBufferSize(), &layout);
				if (d3dResult < 0) {
					LOGE << "Cannot create input layout '" << name << "'";
					return INVALID_RID;
				}
			}
			delete[] descriptors;
			InputLayoutResource* ilr = new InputLayoutResource(layout,index);
			_resCtx->resources.push_back(ilr);
			return create(name, ResourceType::INPUTLAYOUT);
		}

		static bool createVertexShader(ID3DBlob* buffer, ID3D11VertexShader** shader) {
			HRESULT d3dResult = _resCtx->device->CreateVertexShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), 0, shader);
			if (d3dResult < 0) {
				if (buffer) {
					buffer->Release();
				}
				return false;
			}
			return true;
		}

		static bool createGeometryShader(ID3DBlob* buffer, ID3D11GeometryShader** shader) {
			HRESULT d3dResult = _resCtx->device->CreateGeometryShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), 0, shader);
			if (d3dResult < 0) {
				if (buffer) {
					buffer->Release();
				}
				return false;
			}
			return true;
		}

		static bool createPixelShader(ID3DBlob* buffer, ID3D11PixelShader** shader) {
			HRESULT d3dResult = _resCtx->device->CreatePixelShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), 0, shader);
			if (d3dResult < 0) {
				if (buffer) {
					buffer->Release();
				}
				return false;
			}
			return true;
		}

		static bool compileShader(const char* filePath, const  char* entry, const  char* shaderModel, ID3DBlob** buffer) {
			DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined( DEBUG ) || defined( _DEBUG )
			shaderFlags |= D3DCOMPILE_DEBUG;
#endif

			ID3DBlob* errorBuffer = 0;
			HRESULT result;

			 result = D3DX11CompileFromFile(filePath, 0, 0, entry, shaderModel, shaderFlags, 0, 0, buffer, &errorBuffer, 0);

			if (FAILED(result))	{
				if (errorBuffer != 0) {
					LOGE << "Error compiling shader: " << (char*)errorBuffer->GetBufferPointer();
					errorBuffer->Release();
				}
				return false;
			}
			if (errorBuffer != 0) {
				errorBuffer->Release();
			}
			return true;
		}

		RID createEmptyShader(const char* name) {
			Shader* s = new Shader;
			s->vertexShaderBuffer = 0;
			ShaderResource* cbr = new ShaderResource(s);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::SHADER);
		}

		RID createShader(const char* name, const ShaderDescriptor& descriptor) {
			Shader* s = new Shader;
			s->vertexShaderBuffer = 0;
			bool compileResult = compileShader(descriptor.file, descriptor.vertexShader, "vs_4_0", &s->vertexShaderBuffer);
			if (!compileResult)	{
				DXTRACE_MSG("Error compiling the vertex shader!");
				return -1;
			}
			if (!createVertexShader(s->vertexShaderBuffer, &s->vertexShader)) {
				DXTRACE_MSG("Error creating the vertex shader!");
				return -1;
			}
			ID3DBlob* psBuffer = 0;
			compileResult = compileShader(descriptor.file, descriptor.pixelShader, "ps_4_0", &psBuffer);
			if (!compileResult)	{
				DXTRACE_MSG("Error compiling pixel shader!");
				return -1;
			}

			if (!createPixelShader(psBuffer, &s->pixelShader)) {
				DXTRACE_MSG("Error creating pixel shader!");
				return -1;
			}
			psBuffer->Release();
			if (descriptor.geometryShader != 0) {
				ID3DBlob* psBuffer = 0;
				compileResult = compileShader(descriptor.file, descriptor.geometryShader, "gs_4_0", &psBuffer);
				if (!compileResult)	{
					DXTRACE_MSG("Error compiling geometry shader!");
					return -1;
				}

				if (!createGeometryShader(psBuffer, &s->geometryShader)) {
					DXTRACE_MSG("Error creating geometry shader!");
					return -1;
				}
				psBuffer->Release();
			}
			s->samplerState = getSamplerState(descriptor.samplerState);
			ShaderResource* cbr = new ShaderResource(s);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::SHADER);
		}

		// ------------------------------------------------------
		// parse constant buffer
		// ------------------------------------------------------
		void parseConstantBuffer(JSONReader& reader, int childIndex) {
			ConstantBufferDescriptor descriptor;
			reader.get(childIndex, "size", &descriptor.size);
			const char* name = reader.get_string(childIndex, "name");
			createConstantBuffer(name, descriptor);
		}

		// ------------------------------------------------------
		// parse quad index buffer
		// ------------------------------------------------------
		void parseQuadIndexBuffer(JSONReader& reader, int childIndex) {
			QuadIndexBufferDescriptor descriptor;
			reader.get(childIndex, "size", &descriptor.size);
			const char* name = reader.get_string(childIndex, "name");
			createQuadIndexBuffer(name, descriptor);
		}

		// ------------------------------------------------------
		// parse quad index buffer
		// ------------------------------------------------------
		void parseIndexBuffer(JSONReader& reader, int childIndex) {
			IndexBufferDescriptor descriptor;
			reader.get(childIndex, "size", &descriptor.size);
			const char* name = reader.get_string(childIndex, "name");
			createIndexBuffer(name, descriptor);
		}

		// ------------------------------------------------------
		// parse material
		// ------------------------------------------------------
		void parseMaterial(JSONReader& reader, int childIndex) {
			MaterialDescriptor descriptor;
			const char* shaderName = reader.get_string(childIndex, "shader");
			descriptor.shader = find(shaderName, ResourceType::SHADER);
			const char* bsName = reader.get_string(childIndex, "blend_state");
			descriptor.blendstate = find(bsName, ResourceType::BLENDSTATE);
			const char* name = reader.get_string(childIndex, "name");
			reader.get(childIndex, "diffuse", descriptor.diffuse);
			reader.get(childIndex, "ambient", descriptor.ambient);
			if (reader.contains_property(childIndex, "texture")) {
				const char* texName = reader.get_string(childIndex, "texture");
				descriptor.texture = find(texName, ResourceType::TEXTURE);
			}
			else {
				descriptor.texture = INVALID_RID;
			}
			if (reader.contains_property(childIndex, "render_target")) {
				const char* texName = reader.get_string(childIndex, "render_target");
				descriptor.renderTarget = find(texName, ResourceType::RENDERTARGET);
			}
			else {
				descriptor.renderTarget = INVALID_RID;
			}
			createMaterial(name, descriptor);
		}

		// ------------------------------------------------------
		// parse vertex buffer
		// ------------------------------------------------------
		void parseVertexBuffer(JSONReader& reader, int childIndex) {
			VertexBufferDescriptor descriptor;
			reader.get(childIndex, "size", &descriptor.size);
			reader.get(childIndex, "dynamic", &descriptor.dynamic);
			const char* layoutName = reader.get_string(childIndex, "layout");
			descriptor.layout = find(layoutName, ResourceType::INPUTLAYOUT);
			const char* name = reader.get_string(childIndex, "name");
			createVertexBuffer(name, descriptor);
		}

		// ------------------------------------------------------
		// parse shader
		// ------------------------------------------------------
		void parseShader(JSONReader& reader, int childIndex) {
			ShaderDescriptor descriptor;
			descriptor.file = reader.get_string(childIndex, "file");
			descriptor.vertexShader = reader.get_string(childIndex, "vertex_shader");
			descriptor.pixelShader = reader.get_string(childIndex, "pixel_shader");
			descriptor.geometryShader = reader.get_string(childIndex, "geometry_shader");
			descriptor.model = reader.get_string(childIndex, "shader_model");
			const char* samplerStateName = reader.get_string(childIndex, "sampler_state");
			descriptor.samplerState = find(samplerStateName, ResourceType::SAMPLERSTATE);
			const char* name = reader.get_string(childIndex, "name");
			createShader(name, descriptor);
		}

		// ------------------------------------------------------
		// parse blend state
		// ------------------------------------------------------
		void parseBlendState(JSONReader& reader, int childIndex) {
			BlendStateDescriptor descriptor;
			const char* entry = reader.get_string(childIndex, "src_blend");
			descriptor.srcBlend = findBlendState(entry);
			entry = reader.get_string(childIndex, "dest_blend");
			descriptor.destBlend = findBlendState(entry);
			entry = reader.get_string(childIndex, "src_blend_alpha");
			descriptor.srcAlphaBlend = findBlendState(entry);
			entry = reader.get_string(childIndex, "dest_blend_alpha");
			descriptor.destAlphaBlend = findBlendState(entry);
			reader.get(childIndex, "alpha_enabled", &descriptor.alphaEnabled);
			const char* name = reader.get_string(childIndex, "name");
			createBlendState(name, descriptor);
		}

		// ------------------------------------------------------
		// parse texture
		// ------------------------------------------------------
		void parseTexture(JSONReader& reader, int childIndex) {
			TextureDescriptor descriptor;
			descriptor.name = reader.get_string(childIndex, "file");
			const char* name = reader.get_string(childIndex, "name");
			loadTexture(name, descriptor);
		}

		// ------------------------------------------------------
		// parse texture cube
		// ------------------------------------------------------
		void parseTextureCube(JSONReader& reader, int childIndex) {
			TextureDescriptor descriptor;
			descriptor.name = reader.get_string(childIndex, "file");
			const char* name = reader.get_string(childIndex, "name");
			loadTextureCube(name, descriptor);
		}

		// ------------------------------------------------------
		// parse scene
		// ------------------------------------------------------
		void parseScene(JSONReader& reader, int childIndex) {
			SceneDescriptor descriptor;
			reader.get(childIndex, "size", &descriptor.size);
			descriptor.meshBuffer = reader.get_string(childIndex, "mesh_buffer");
			descriptor.camera = reader.get_string(childIndex, "camera");
			reader.get(childIndex, "depth_enabled", &descriptor.depthEnabled);
			const char* name = reader.get_string(childIndex, "name");
			createScene(name, descriptor);
		}

		// ------------------------------------------------------
		// parse input layout
		// ------------------------------------------------------
		void parseInputLayout(JSONReader& reader, int childIndex) {
			InputLayoutDescriptor descriptor;
			descriptor.num = 0;
			const char* attributes = reader.get_string(childIndex, "attributes");
			char buffer[256];
			sprintf_s(buffer, 256, "%s", attributes);
			char *token = token = strtok(buffer, ",");
			while (token) {
				int element = findInputElement(token);
				descriptor.indices[descriptor.num++] = element;
				token = strtok(NULL, ",");
			}
			const char* shaderName = reader.get_string(childIndex, "shader");
			descriptor.shader = find(shaderName, ResourceType::SHADER);
			const char* name = reader.get_string(childIndex, "name");
			createInputLayout(name, descriptor);
		}

		// ------------------------------------------------------
		// parse font
		// ------------------------------------------------------
		void parseFont(JSONReader& reader, int childIndex) {
			BitmapfontDescriptor descriptor;
			descriptor.name = reader.get_string(childIndex, "file");
			const char* name = reader.get_string(childIndex, "name");
			loadFont(name, descriptor);
		}

		// ------------------------------------------------------
		// parse particle manager
		// ------------------------------------------------------
		void parseParticleManager(JSONReader& reader, int childIndex) {			
			ParticleSystemsDescriptor descriptor;
			reader.get(childIndex, "sprite_buffer", &descriptor.spriteBuffer);
			const char* name = reader.get_string(childIndex, "name");
			createParticleManager(descriptor);
		}

		// ------------------------------------------------------
		// parse sampler state
		// ------------------------------------------------------
		void parseSamplerState(JSONReader& reader, int childIndex) {
			SamplerStateDescriptor descriptor;
			const char* mode = reader.get_string(childIndex, "addressU");
			int idx = findTextureAddressMode(mode);
			XASSERT(idx >= 0,"No matching address mode for addressU mode: '%s'",mode);
			descriptor.addressU = idx;
			mode = reader.get_string(childIndex, "addressV");
			idx = findTextureAddressMode(mode);
			XASSERT(idx >= 0, "No matching address mode for addressV mode: '%s'", mode);
			descriptor.addressV = idx;
			mode = reader.get_string(childIndex, "addressW");
			idx = findTextureAddressMode(mode);
			XASSERT(idx >= 0, "No matching address mode for addressW mode: '%s'", mode);
			descriptor.addressW = idx;
			const char* name = reader.get_string(childIndex, "name");
			createSamplerState(name, descriptor);
		}

		// ------------------------------------------------------
		// parse sprite buffer
		// ------------------------------------------------------
		void parseSpriteBuffer(JSONReader& reader, int childIndex) {
			SpriteBufferDescriptor descriptor;
			reader.get(childIndex, "size", &descriptor.size);
			reader.get(childIndex, "index_buffer", &descriptor.indexBuffer);
			const char* constantBufferName = reader.get_string(childIndex, "constant_buffer");
			descriptor.constantBuffer = find(constantBufferName, ResourceType::CONSTANTBUFFER);
			const char* vertexBufferName = reader.get_string(childIndex, "vertex_buffer");
			descriptor.vertexBuffer = find(vertexBufferName, ResourceType::VERTEXBUFFER);
			const char* materialName = reader.get_string(childIndex, "material");
			descriptor.material = find(materialName, ResourceType::MATERIAL);
			if (reader.contains_property(childIndex, "font")) {
				const char* fontName = reader.get_string(childIndex, "font");
				descriptor.font = find(fontName, ResourceType::BITMAPFONT);
			}
			else {
				descriptor.font = INVALID_RID;
			}
			const char* name = reader.get_string(childIndex, "name");
			createSpriteBuffer(name, descriptor);
		}

		// ------------------------------------------------------
		// parse quad buffer
		// ------------------------------------------------------
		void parseQuadBuffer(JSONReader& reader, int childIndex) {
			QuadBufferDescriptor descriptor;
			reader.get(childIndex, "size", &descriptor.size);
			reader.get(childIndex, "index_buffer", &descriptor.indexBuffer);
			reader.get(childIndex, "constant_buffer", &descriptor.constantBuffer);
			reader.get(childIndex, "vertex_buffer", &descriptor.vertexBuffer);
			reader.get(childIndex, "shader", &descriptor.shader);
			reader.get(childIndex, "blend_state", &descriptor.blendstate);
			reader.get(childIndex, "color_map", &descriptor.colormap);
			reader.get(childIndex, "input_layout", &descriptor.inputlayout);
			const char* name = reader.get_string(childIndex, "name");
			createQuadBuffer(name, descriptor);
		}

		/*
		mesh_buffer {
		color_map : 5
		}
		*/
		// ------------------------------------------------------
		// parse Mesh buffer
		// ------------------------------------------------------
		void parseMeshBuffer(JSONReader& reader, int childIndex) {
			MeshBufferDescriptor descriptor;
			reader.get(childIndex, "size", &descriptor.size);
			const char* indexBufferName = reader.get_string(childIndex, "index_buffer");
			descriptor.indexBuffer = find(indexBufferName, ResourceType::INDEXBUFFER);
			const char* constantBufferName = reader.get_string(childIndex, "constant_buffer");			
			descriptor.constantBuffer = find(constantBufferName, ResourceType::CONSTANTBUFFER);
			const char* vertexBufferName = reader.get_string(childIndex, "vertex_buffer");
			descriptor.vertexBuffer = find(vertexBufferName, ResourceType::VERTEXBUFFER);
			const char* materialName = reader.get_string(childIndex, "material");
			descriptor.material = find(materialName, ResourceType::MATERIAL);
			const char* name = reader.get_string(childIndex, "name");
			createMeshBuffer(name, descriptor);
		}

		// ------------------------------------------------------
		// parse Mesh buffer
		// ------------------------------------------------------
		void parseSkyBox(JSONReader& reader, int childIndex) {
			SkyBoxDescriptor descriptor;
			reader.get(childIndex, "scale", &descriptor.scale);
			const char* indexBufferName = reader.get_string(childIndex, "index_buffer");
			descriptor.indexBuffer = find(indexBufferName, ResourceType::INDEXBUFFER);
			const char* constantBufferName = reader.get_string(childIndex, "constant_buffer");
			descriptor.constantBuffer = find(constantBufferName, ResourceType::CONSTANTBUFFER);
			const char* vertexBufferName = reader.get_string(childIndex, "vertex_buffer");
			descriptor.vertexBuffer = find(vertexBufferName, ResourceType::VERTEXBUFFER);
			const char* shaderName = reader.get_string(childIndex, "shader");
			descriptor.shader = find(shaderName, ResourceType::SHADER);
			const char* blendStateName = reader.get_string(childIndex, "blend_state");
			descriptor.blendstate = find(blendStateName, ResourceType::BLENDSTATE);
			reader.get(childIndex, "color_map", &descriptor.colormap);
			const char* inputLayoutName = reader.get_string(childIndex, "input_layout");
			descriptor.inputlayout = find(inputLayoutName, ResourceType::INPUTLAYOUT);
			const char* name = reader.get_string(childIndex, "name");
			createSkyBox(name, descriptor);
		}

		// ------------------------------------------------------
		// parse rendertarget
		// ------------------------------------------------------
		void parseRenderTarget(JSONReader& reader, int childIndex) {
			RenderTargetDescriptor descriptor;
			reader.get(childIndex, "width", &descriptor.width);
			reader.get(childIndex, "height", &descriptor.height);
			reader.get(childIndex, "clear_color", &descriptor.clearColor);
			const char* name = reader.get_string(childIndex, "name");
			createRenderTarget(name, descriptor);
		}

		// ------------------------------------------------------
		// parse Mesh
		// ------------------------------------------------------
		void parseMesh(JSONReader& reader, int childIndex) {
			MeshDescriptor descriptor;
			reader.get(childIndex, "position", &descriptor.position);
			reader.get(childIndex, "scale", &descriptor.scale);
			reader.get(childIndex, "rotation", &descriptor.rotation);
			const char* name = reader.get_string(childIndex, "name");
			descriptor.fileName = name;
			createMesh(name, descriptor);
		}

		// ------------------------------------------------------
		// parse IMGUI
		// ------------------------------------------------------
		void parseIMGUI(JSONReader& reader, int childIndex) {
			IMGUIDescriptor descriptor;
			const char* spriteBufferName = reader.get_string(childIndex, "sprite_buffer");
			descriptor.spriteBuffer = find(spriteBufferName, ResourceType::SPRITEBUFFER);
			const char* fontName = reader.get_string(childIndex, "font");
			descriptor.font = find(fontName, ResourceType::BITMAPFONT);
			gui::initialize(descriptor);
		}

		// ------------------------------------------------------
		// parse dialog
		// ------------------------------------------------------
		void parseDialog(JSONReader& reader, int childIndex) {
			GUIDialogDescriptor descriptor;
			const char* fontName = reader.get_string(childIndex, "font");
			descriptor.font = find(fontName, ResourceType::BITMAPFONT);
			descriptor.file = reader.get_string(childIndex, "file");
			const char* name = reader.get_string(childIndex, "name");
			createDialog(name, descriptor);
		}



		// ------------------------------------------------------
		// initialize
		// ------------------------------------------------------
		void initialize(ID3D11Device* device) {
			_resCtx = new ResourceContext;
			_resCtx->device = device;
			_resCtx->resourceIndex = 0;				
			_resCtx->particles = 0;
			_resCtx->parsers[string::murmur_hash("constant_buffer")] = parseConstantBuffer;
			_resCtx->parsers[string::murmur_hash("quad_index_buffer")] = parseQuadIndexBuffer;
			_resCtx->parsers[string::murmur_hash("index_buffer")] = parseIndexBuffer;
			_resCtx->parsers[string::murmur_hash("dialog")] = parseDialog;
			_resCtx->parsers[string::murmur_hash("sprite_buffer")] = parseSpriteBuffer;
			_resCtx->parsers[string::murmur_hash("shader")] = parseShader;
			_resCtx->parsers[string::murmur_hash("sampler_state")] = parseSamplerState;
			_resCtx->parsers[string::murmur_hash("input_layout")] = parseInputLayout;
			_resCtx->parsers[string::murmur_hash("vertex_buffer")] = parseVertexBuffer;
			_resCtx->parsers[string::murmur_hash("font")] = parseFont;
			_resCtx->parsers[string::murmur_hash("texture")] = parseTexture;
			_resCtx->parsers[string::murmur_hash("blendstate")] = parseBlendState;
			_resCtx->parsers[string::murmur_hash("imgui")] = parseIMGUI;
			_resCtx->parsers[string::murmur_hash("mesh")] = parseMesh;
			_resCtx->parsers[string::murmur_hash("mesh_buffer")] = parseMeshBuffer;
			_resCtx->parsers[string::murmur_hash("quad_buffer")] = parseQuadBuffer;
			_resCtx->parsers[string::murmur_hash("scene")] = parseScene;
			_resCtx->parsers[string::murmur_hash("texture_cube")] = parseTextureCube;
			_resCtx->parsers[string::murmur_hash("skybox")] = parseSkyBox;
			_resCtx->parsers[string::murmur_hash("material")] = parseMaterial;
			_resCtx->parsers[string::murmur_hash("particle_manager")] = parseParticleManager;
			_resCtx->parsers[string::murmur_hash("render_target")] = parseRenderTarget;
		}

		// ------------------------------------------------------
		// parse specific json file
		// ------------------------------------------------------
		void parseJSONFile(const char* fileName) {
			JSONReader reader;
			char buffer[256];
			IdString importHash = string::murmur_hash("import");
			sprintf_s(buffer, 256, "content\\%s", fileName);
			LOG << "Loading resource file: " << fileName;
			bool ret = reader.parse(buffer);
			XASSERT(ret, "Failed parsing json file");
			int children[256];
			int num = reader.get_categories(children, 256);
			for (int i = 0; i < num; ++i) {
				IdString hash = string::murmur_hash(reader.get_category_name(i));
				// special category to import other files
				if (hash == importHash) {
					const char* fileName = reader.get_string(children[i], "file");
					parseJSONFile(fileName);
				}
				else {
					if (_resCtx->parsers.find(hash) != _resCtx->parsers.end()) {
						ParseFunc f = _resCtx->parsers[hash];
						LOG << "Parsing '" << reader.get_category_name(i) << "'";
						(*f)(reader, i);
					}
					else {
						LOG << "No matching parser for '" << reader.get_category_name(i) << "'";
					}
				}
			}
		}

		// ------------------------------------------------------
		// parse json file
		// ------------------------------------------------------
		void parseJSONFile() {
			parseJSONFile("resources.json");
		}

		uint32_t getIndex(RID rid, ResourceType type) {
			const ResourceIndex& res_idx = _resCtx->indices[rid];
			XASSERT(res_idx.type == type, "Different resource types - expected %s but found %s", ResourceTypeNames[res_idx.type], ResourceTypeNames[type]);
			return res_idx.id;
		}

		ID3D11Buffer* getIndexBuffer(RID rid) {
			const ResourceIndex& res_idx = _resCtx->indices[rid];
			XASSERT(res_idx.type == ResourceType::INDEXBUFFER, "Different resource types - expected INDEXBUFFER but found %s", ResourceTypeNames[res_idx.type]);
			IndexBufferResource* res = static_cast<IndexBufferResource*>(_resCtx->resources[res_idx.id]);
			return res->get();
		}

		ID3D11BlendState* getBlendState(RID rid) {
			const ResourceIndex& res_idx = _resCtx->indices[rid];
			XASSERT(res_idx.type == ResourceType::BLENDSTATE, "Different resource types - expected BLENDSTATE but found %s", ResourceTypeNames[res_idx.type]);
			BlendStateResource* res = static_cast<BlendStateResource*>(_resCtx->resources[res_idx.id]);
			return res->get();
		}

		bool contains(RID rid, ResourceType type) {
			const ResourceIndex& res_idx = _resCtx->indices[rid];
			return res_idx.type == type;
		}

		RID find(const char* name, ResourceType type) {
			IdString hash = string::murmur_hash(name);
			int idx = -1;
			for (uint32_t i = 0; i < _resCtx->indices.size(); ++i) {
				if (_resCtx->indices[i].hash == hash) {
					idx = i;
				}
			}
			XASSERT(idx != -1, "No matching resource found for: %s", name);
			const ResourceIndex& res_idx = _resCtx->indices[idx];
			XASSERT(res_idx.type == type, "Different resource types - expected %s but found %s", ResourceTypeNames[res_idx.type],ResourceTypeNames[type]);
			return res_idx.id;
		}

		ID3D11Buffer* getConstantBuffer(const char* name) {
			RID rid = find(name, ResourceType::CONSTANTBUFFER);
			ConstantBufferResource* res = static_cast<ConstantBufferResource*>(_resCtx->resources[rid]);
			return res->get();			
		}

		ID3D11Buffer* getConstantBuffer(RID rid) {
			const ResourceIndex& res_idx = _resCtx->indices[rid];
			XASSERT(res_idx.type == ResourceType::CONSTANTBUFFER, "Different resource types - expected CONSTANTBUFFER but found %s", ResourceTypeNames[res_idx.type]);
			ConstantBufferResource* res = static_cast<ConstantBufferResource*>(_resCtx->resources[res_idx.id]);
			return res->get();
		}

		ID3D11Buffer* getVertexBuffer(RID rid) {
			const ResourceIndex& res_idx = _resCtx->indices[rid];
			XASSERT(res_idx.type == ResourceType::VERTEXBUFFER, "Different resource types - expected VERTEXBUFFER but found %s", ResourceTypeNames[res_idx.type]);
			VertexBufferResource* res = static_cast<VertexBufferResource*>(_resCtx->resources[res_idx.id]);
			return res->get();
		}

		ID3D11InputLayout* getInputLayout(const char* name) {
			RID rid = find(name,ResourceType::INPUTLAYOUT);
			InputLayoutResource* res = static_cast<InputLayoutResource*>(_resCtx->resources[rid]);
			return res->get();			
		}

		ID3D11InputLayout* getInputLayout(RID rid) {
			const ResourceIndex& res_idx = _resCtx->indices[rid];
			XASSERT(res_idx.type == ResourceType::INPUTLAYOUT, "Different resource types - expected INPUTLAYOUT but found %s", ResourceTypeNames[res_idx.type]);
			InputLayoutResource* res = static_cast<InputLayoutResource*>(_resCtx->resources[res_idx.id]);
			return res->get();
		}

		ID3D11ShaderResourceView* getShaderResourceView(RID rid) {
			const ResourceIndex& res_idx = _resCtx->indices[rid];
			XASSERT(res_idx.type == ResourceType::TEXTURE, "Different resource types - expected TEXTURE but found %s", ResourceTypeNames[res_idx.type]);
			ShaderResourceViewResource* res = static_cast<ShaderResourceViewResource*>(_resCtx->resources[res_idx.id]);
			return res->get();
		}

		ID3D11SamplerState* getSamplerState(RID rid) {
			const ResourceIndex& res_idx = _resCtx->indices[rid];
			XASSERT(res_idx.type == ResourceType::SAMPLERSTATE, "Different resource types - expected SAMPLERSTATE but found %s", ResourceTypeNames[res_idx.type]);
			SamplerStateResource* res = static_cast<SamplerStateResource*>(_resCtx->resources[res_idx.id]);
			return res->get();
		}

		Shader* getShader(RID rid) {
			const ResourceIndex& res_idx = _resCtx->indices[rid];
			XASSERT(res_idx.type == ResourceType::SHADER, "Different resource types - expected SHADER but found %s", ResourceTypeNames[res_idx.type]);
			ShaderResource* res = static_cast<ShaderResource*>(_resCtx->resources[res_idx.id]);
			return res->get();
		}

		Bitmapfont* getFont(RID rid) {
			const ResourceIndex& res_idx = _resCtx->indices[rid];
			XASSERT(res_idx.type == ResourceType::BITMAPFONT, "Different resource types - expected BITMAPFONT but found %s", ResourceTypeNames[res_idx.type]);
			BitmapfontResource* res = static_cast<BitmapfontResource*>(_resCtx->resources[res_idx.id]);
			return res->get();
		}

		SpriteBuffer* getSpriteBuffer(const char* name) {
			RID rid = find(name,ResourceType::SPRITEBUFFER);
			SpriteBufferResource* res = static_cast<SpriteBufferResource*>(_resCtx->resources[rid]);
			return res->get();			
		}

		SpriteBuffer* getSpriteBuffer(RID rid) {
			const ResourceIndex& res_idx = _resCtx->indices[rid];
			XASSERT(res_idx.type == ResourceType::SPRITEBUFFER, "Different resource types - expected SPRITEBUFFER but found %s", ResourceTypeNames[res_idx.type]);
			SpriteBufferResource* res = static_cast<SpriteBufferResource*>(_resCtx->resources[res_idx.id]);
			return res->get();
		}

		Mesh* getMesh(RID rid) {
			const ResourceIndex& res_idx = _resCtx->indices[rid];
			XASSERT(res_idx.type == ResourceType::MESH, "Different resource types - expected MESH but found %s", ResourceTypeNames[res_idx.type]);
			MeshResource* res = static_cast<MeshResource*>(_resCtx->resources[res_idx.id]);
			return res->get();
		}

		Mesh* getMesh(const char* name) {
			RID rid = find(name, ResourceType::MESH);
			MeshResource* res = static_cast<MeshResource*>(_resCtx->resources[rid]);
			return res->get();
		}

		RenderTarget* getRenderTarget(RID rid) {
			const ResourceIndex& res_idx = _resCtx->indices[rid];
			XASSERT(res_idx.type == ResourceType::RENDERTARGET, "Different resource types - expected RENDERTARGET but found %s", ResourceTypeNames[res_idx.type]);
			RenderTargetResource* res = static_cast<RenderTargetResource*>(_resCtx->resources[rid]);
			return res->get();
		}

		Material* getMaterial(const char* name) {
			RID rid = find(name, ResourceType::MATERIAL);
			MaterialResource* res = static_cast<MaterialResource*>(_resCtx->resources[rid]);
			return res->get();
		}

		Material* getMaterial(RID rid) {
			const ResourceIndex& res_idx = _resCtx->indices[rid];
			XASSERT(res_idx.type == ResourceType::MATERIAL, "Different resource types - expected MATERIAL but found %s", ResourceTypeNames[res_idx.type]);
			MaterialResource* res = static_cast<MaterialResource*>(_resCtx->resources[res_idx.id]);
			return res->get();
		}

		SkyBox* getSkyBox(const char* name) {
			RID rid = find(name, ResourceType::SKYBOX);
			SkyBoxResource* res = static_cast<SkyBoxResource*>(_resCtx->resources[rid]);
			return res->get();
		}

		MeshBuffer* getMeshBuffer(RID rid) {
			const ResourceIndex& res_idx = _resCtx->indices[rid];
			XASSERT(res_idx.type == ResourceType::MESHBUFFER, "Different resource types - expected MESHBUFFER but found %s", ResourceTypeNames[res_idx.type]);
			MeshBufferResource* res = static_cast<MeshBufferResource*>(_resCtx->resources[res_idx.id]);
			return res->get();
		}

		MeshBuffer* getMeshBuffer(const char* name) {
			RID rid = find(name, ResourceType::MESHBUFFER);
			MeshBufferResource* res = static_cast<MeshBufferResource*>(_resCtx->resources[rid]);
			return res->get();
		}

		Scene* getScene(const char* name) {
			RID rid = find(name, ResourceType::SCENE);
			SceneResource* res = static_cast<SceneResource*>(_resCtx->resources[rid]);
			return res->get();
		}
		
		QuadBuffer* getQuadBuffer(RID rid) {
			const ResourceIndex& res_idx = _resCtx->indices[rid];
			XASSERT(res_idx.type == ResourceType::QUADBUFFER, "Different resource types - expected QUADBUFFER but found %s", ResourceTypeNames[res_idx.type]);
			QuadBufferResource* res = static_cast<QuadBufferResource*>(_resCtx->resources[res_idx.id]);
			return res->get();
		}

		void reloadDialog(const char* name) {
			LOG << "reloading: " << name;
			RID rid = find(name, ResourceType::GUIDIALOG);
			GUIDialogResource* res = static_cast<GUIDialogResource*>(_resCtx->resources[rid]);
			GUIDialog* dlg = res->get();
			dlg->load();
		}

		GUIDialog* getGUIDialog(RID rid) {
			const ResourceIndex& res_idx = _resCtx->indices[rid];
			XASSERT(res_idx.type == ResourceType::GUIDIALOG, "Different resource types - expected GUIDIALOG but found %s", ResourceTypeNames[res_idx.type]);
			GUIDialogResource* res = static_cast<GUIDialogResource*>(_resCtx->resources[res_idx.id]);
			return res->get();
		}

		GUIDialog* getGUIDialog(const char* name) {
			RID rid = find(name, ResourceType::GUIDIALOG);
			GUIDialogResource* res = static_cast<GUIDialogResource*>(_resCtx->resources[rid]);
			return res->get();
		}

		BaseResource* getResource(RID rid, ResourceType type) {
			const ResourceIndex& res_idx = _resCtx->indices[rid];
			XASSERT(res_idx.type == type, "Different resource types - expected %s but found %s", ResourceTypeNames[type],ResourceTypeNames[res_idx.type]);
			return _resCtx->resources[res_idx.id];
		}

		ParticleManager* getParticleManager() {
			return _resCtx->particles;
		}

		void debug() {
			for (uint32_t i = 0; i < _resCtx->indices.size(); ++i) {
				ResourceIndex& index = _resCtx->indices[i];
				if (index.type != ResourceType::UNKNOWN) {
					if (index.nameIndex != -1) {
						const char* text = _resCtx->nameBuffer.data + index.nameIndex;
						LOG << index.id << " Type: " << ResourceTypeNames[index.type] << " name: " << text;
					}
				}
			}
		}

		// -------------------------------------------------------------
		// get name
		// -------------------------------------------------------------
		const char* getName(RID rid) {
			const ResourceIndex& res_idx = _resCtx->indices[rid];
			return _resCtx->nameBuffer.data + res_idx.nameIndex;
		}

		// -------------------------------------------------------------
		// save report
		// -------------------------------------------------------------
		void save(const ReportWriter& writer) {
			writer.startBox("Resources");
			const char* HEADERS[] = { "ID", "Type", "Name" };
			writer.startTable(HEADERS, 3);
			for (uint32_t i = 0; i < _resCtx->resources.size(); ++i) {
				ResourceIndex& index = _resCtx->indices[i];
				if (index.type != ResourceType::UNKNOWN) {
					if (index.nameIndex != -1) {
						writer.startRow();
						writer.addCell(index.id);
						writer.addCell(ResourceTypeNames[index.type]);
						const char* text = _resCtx->nameBuffer.data + index.nameIndex;
						writer.addCell(text);
						writer.endRow();
					}
				}
			}
			writer.endTable();
			writer.endBox();			
		}

	}
}