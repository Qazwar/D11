#include "ResourceContainer.h"
#include <D3D11.h>
#include <D3Dcompiler.h>
#include "..\io\json.h"
#include "ResourceDescriptors.h"
#include "..\renderer\graphics.h"
#include <string.h>
#include "..\utils\Log.h"
#include "..\renderer\render_types.h"
#include "..\imgui\IMGUI.h"
#include "..\utils\ObjLoader.h"
#include "Resource.h"
#include "..\renderer\RenderTarget.h"
#include "..\renderer\SkyBox.h"

namespace ds {

	namespace res {	

		typedef void(*ParseFunc)(JSONReader&, int);

		// ------------------------------------------------------
		// resource index
		// ------------------------------------------------------
		struct ResourceIndex {
			RID id;
			uint32_t index;
			ResourceType type;
			uint32_t nameIndex;
		};

		// ------------------------------------------------------
		// resource context
		// ------------------------------------------------------
		struct ResourceContext {

			CharBuffer nameBuffer;
			ID3D11Device* device;
			uint32_t resourceIndex;
			ResourceIndex resourceTable[MAX_RESOURCES];
			ParticleManager* particles;

			std::vector<BaseResource*> resources;
			std::map<IdString, ResourceIndex> lookup;

			std::map<IdString, ParseFunc> parsers;
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
			"TEXTURECUBE",
			"SKYBOX",
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
		static int findBlendState(const char* text) {
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
			if (_resCtx->particles != 0) {
				delete _resCtx->particles;
			}
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
		static RID create(ResourceIndex& ri, const char* name, uint16_t id, int index, ResourceType type) {
			IdString hash = string::murmur_hash(name);
			ri.index = index;
			ri.id = id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = type;
			_resCtx->lookup[hash] = ri;
			return ri.id;
		}

		// ------------------------------------------------------
		// cretate quad index buffer
		// ------------------------------------------------------
		static RID createQuadIndexBuffer(const char* name,const QuadIndexBufferDescriptor& descriptor) {
			int idx = _resCtx->resources.size();
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
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
			IndexBufferResource* cbr = new IndexBufferResource(buffer);
			_resCtx->resources.push_back(cbr);
			IdString hash = string::murmur_hash(name);
			ri.index = idx;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::INDEXBUFFER;
			_resCtx->lookup[hash] = ri;
			return ri.id;
		}

		// ------------------------------------------------------
		// create index buffer
		// ------------------------------------------------------
		static RID createIndexBuffer(const char* name, const IndexBufferDescriptor& descriptor) {
			int idx = _resCtx->resources.size();
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
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
			IdString hash = string::murmur_hash(name);
			ri.index = idx;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::INDEXBUFFER;
			_resCtx->lookup[hash] = ri;
			return ri.id;
		}

		// ------------------------------------------------------
		// create constant buffer
		// ------------------------------------------------------
		static RID createConstantBuffer(const char* name, const ConstantBufferDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			int index = _resCtx->resources.size();
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
			IdString hash = string::murmur_hash(name);
			//_resCtx->constantBuffers.push_back(buffer);
			ri.index = index;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::CONSTANTBUFFER;
			_resCtx->lookup[hash] = ri;
			return ri.id;
		}

		// ------------------------------------------------------
		// create render target
		// ------------------------------------------------------
		static RID createRenderTarget(const char* name, const RenderTargetDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			int index = _resCtx->resources.size();
			RenderTarget* rt= new RenderTarget(descriptor);
			RenderTargetResource* cbr = new RenderTargetResource(rt);
			_resCtx->resources.push_back(cbr);
			IdString hash = string::murmur_hash(name);
			ri.index = index;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::RENDERTARGET;
			_resCtx->lookup[hash] = ri;
			return ri.id;
		}

		// ------------------------------------------------------
		// create sprite buffer
		// ------------------------------------------------------
		static RID createSpriteBuffer(const char* name, const SpriteBufferDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			int index = _resCtx->resources.size();
			SpriteBuffer* buffer = new SpriteBuffer(descriptor);
			SpriteBufferResource* cbr = new SpriteBufferResource(buffer);
			_resCtx->resources.push_back(cbr);
			IdString hash = string::murmur_hash(name);
			ri.index = index;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::SPRITEBUFFER;
			_resCtx->lookup[hash] = ri;
			return ri.id;
		}

		// ------------------------------------------------------
		// create quad buffer
		// ------------------------------------------------------
		static RID createQuadBuffer(const char* name, const QuadBufferDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			int index = _resCtx->resources.size();
			QuadBuffer* buffer = new QuadBuffer(descriptor);
			QuadBufferResource* cbr = new QuadBufferResource(buffer);
			_resCtx->resources.push_back(cbr);
			IdString hash = string::murmur_hash(name);
			ri.index = index;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::QUADBUFFER;
			_resCtx->lookup[hash] = ri;
			return ri.id;
		}

		// ------------------------------------------------------
		// create mesh buffer
		// ------------------------------------------------------
		static RID createMeshBuffer(const char* name, const MeshBufferDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			int index = _resCtx->resources.size();
			MeshBuffer* buffer = new MeshBuffer(descriptor);
			MeshBufferResource* cbr = new MeshBufferResource(buffer);
			_resCtx->resources.push_back(cbr);
			IdString hash = string::murmur_hash(name);
			ri.index = index;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::MESHBUFFER;
			_resCtx->lookup[hash] = ri;
			return ri.id;
		}

		

		// ------------------------------------------------------
		// create skybox
		// ------------------------------------------------------
		static RID createSkyBox(const char* name, const SkyBoxDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			int index = _resCtx->resources.size();
			SkyBox* buffer = new SkyBox(descriptor);
			SkyBoxResource* cbr = new SkyBoxResource(buffer);
			_resCtx->resources.push_back(cbr);
			return create(ri, name, descriptor.id, index, ResourceType::SKYBOX);
			/*
			IdString hash = string::murmur_hash(name);
			ri.index = index;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::SKYBOX;
			_resCtx->lookup[hash] = ri;
			return ri.id;
			*/
		}

		// ------------------------------------------------------
		// create mesh 
		// ------------------------------------------------------
		static RID createMesh(const char* name, const MeshDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			int index = _resCtx->resources.size();
			Mesh* mesh = new Mesh;
			mesh->load(descriptor.fileName);
			MeshResource* cbr = new MeshResource(mesh);
			_resCtx->resources.push_back(cbr);
			IdString hash = string::murmur_hash(name);
			ri.index = index;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::MESH;
			_resCtx->lookup[hash] = ri;
			return ri.id;
		}

		

		// ------------------------------------------------------
		// create sampler state
		// ------------------------------------------------------
		RID createSamplerState(const char* name, const SamplerStateDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			int index = _resCtx->resources.size();
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
			IdString hash = string::murmur_hash(name);
			ri.index = index;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::SAMPLERSTATE;
			_resCtx->lookup[hash] = ri;
			return ri.id;
		}

		// ------------------------------------------------------
		// load texture
		// ------------------------------------------------------
		static RID loadTexture(const char* name, const TextureDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			int idx = _resCtx->resources.size();
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
			IdString hash = string::murmur_hash(name);			
			ri.index = idx;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::TEXTURE;
			_resCtx->lookup[hash] = ri;
			return ri.id;
		}

		// ------------------------------------------------------
		// load texture cube
		// ------------------------------------------------------
		static RID loadTextureCube(const char* name, const TextureDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			int idx = _resCtx->resources.size();
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
			IdString hash = string::murmur_hash(name);
			ri.index = idx;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::TEXTURE;
			_resCtx->lookup[hash] = ri;
			return ri.id;
		}

		// ------------------------------------------------------
		// load bitmap font
		// ------------------------------------------------------
		static RID loadFont(const char* name,const BitmapfontDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			int idx = _resCtx->resources.size();

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
			IdString hash = string::murmur_hash(name);
			ri.index = idx;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::BITMAPFONT;
			_resCtx->lookup[hash] = ri;
			return ri.id;
		}

		// ------------------------------------------------------
		// create blend state
		// ------------------------------------------------------
		static RID createBlendState(const char* name, const BlendStateDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			int idx = _resCtx->resources.size();
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
			IdString hash = string::murmur_hash(name);			
			ri.index = idx;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::BLENDSTATE;
			_resCtx->lookup[hash] = ri;
			return ri.id;
		}

		// ------------------------------------------------------
		// create world
		// ------------------------------------------------------
		static RID createWorld(const char* name, const WorldDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			World* w = new World(descriptor);
			int idx = _resCtx->resources.size();
			WorldResource* cbr = new WorldResource(w);
			_resCtx->resources.push_back(cbr);
			IdString hash = string::murmur_hash(name);
			ri.index = idx;
			ri.id = descriptor.id;
			ri.type = ResourceType::WORLD;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			_resCtx->lookup[hash] = ri;
			return ri.id;
		}

		// ------------------------------------------------------
		// create camera
		// ------------------------------------------------------
		static RID createCamera(const char* name, const CameraDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			CameraResource* cbr = 0;
			if (strcmp(descriptor.type, "orthographic") == 0) {
				OrthoCamera* camera = new OrthoCamera(graphics::getScreenWidth(), graphics::getScreenHeight());
				cbr = new CameraResource(camera);
			}
			else if (strcmp(descriptor.type, "fps") == 0) {
				FPSCamera* camera = new FPSCamera(graphics::getScreenWidth(), graphics::getScreenHeight());
				camera->setPosition(descriptor.position, descriptor.target);
				camera->resetPitch(0.0f);
				camera->resetYAngle();
				cbr = new CameraResource(camera);
			}
			int idx = _resCtx->resources.size();
			_resCtx->resources.push_back(cbr);
			IdString hash = string::murmur_hash(name);
			ri.index = idx;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::CAMERA;
			_resCtx->lookup[hash] = ri;
			return ri.id;
		}

		// ------------------------------------------------------
		// create scene
		// ------------------------------------------------------
		static RID createScene(const char* name, const SceneDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			Scene* scene = new Scene(descriptor);
			int idx = _resCtx->resources.size();
			SceneResource* cbr = new SceneResource(scene);
			_resCtx->resources.push_back(cbr);
			IdString hash = string::murmur_hash(name);
			ri.index = idx;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::SCENE;
			_resCtx->lookup[hash] = ri;
			return ri.id;
		}

		// ------------------------------------------------------
		// create dialog
		// ------------------------------------------------------
		static RID createDialog(const char* name, const GUIDialogDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			GUIDialog* dialog = new GUIDialog(descriptor);
			dialog->load();
			int idx = _resCtx->resources.size();
			GUIDialogResource* cbr = new GUIDialogResource(dialog);
			_resCtx->resources.push_back(cbr);
			IdString hash = string::murmur_hash(name);
			ri.index = idx;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::GUIDIALOG;
			_resCtx->lookup[hash] = ri;
			return ri.id;
		}

		// ------------------------------------------------------
		// create particle manager
		// ------------------------------------------------------
		static void createParticleManager(const ParticleSystemsDescriptor& descriptor) {
			_resCtx->particles = new ParticleManager(descriptor);
			_resCtx->particles->load();
		}

		// ------------------------------------------------------
		// create vertex buffer
		// ------------------------------------------------------
		static RID createVertexBuffer(const char* name, const VertexBufferDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			

			const ResourceIndex& lyt_idx = _resCtx->resourceTable[descriptor.layout];
			assert(lyt_idx.type == ResourceType::INPUTLAYOUT);
			InputLayoutResource* res = static_cast<InputLayoutResource*>(_resCtx->resources[lyt_idx.index]);
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
			VertexBufferResource* cbr = new VertexBufferResource(buffer, size);
			_resCtx->resources.push_back(cbr);
			IdString hash = string::murmur_hash(name);
			ri.index = idx;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::VERTEXBUFFER;
			_resCtx->lookup[hash] = ri;
			LOG << "VertexBuffer '" << name << "' id: " << ri.id << " size: " << size;
			return ri.id;
		}

		// ------------------------------------------------------
		// create vertex buffer
		// ------------------------------------------------------
		static RID createInputLayout(const char* name, const InputLayoutDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			int idx = _resCtx->resources.size();
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
			const ResourceIndex& res_idx = _resCtx->resourceTable[descriptor.shader];
			assert(res_idx.type == ResourceType::SHADER);
			ShaderResource* sr = static_cast<ShaderResource*>(_resCtx->resources[res_idx.index]);
			Shader* s = sr->get();
			assert(s != 0);
			HRESULT d3dResult = _resCtx->device->CreateInputLayout(descriptors, descriptor.num, s->vertexShaderBuffer->GetBufferPointer(), s->vertexShaderBuffer->GetBufferSize(), &layout);
			if (d3dResult < 0) {
				LOGE << "Cannot create input layout '" << name << "'";
				return INVALID_RID;
			}
			delete[] descriptors;
			InputLayoutResource* ilr = new InputLayoutResource(layout,index);
			_resCtx->resources.push_back(ilr);
			ri.index = idx;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::INPUTLAYOUT;
			IdString hash = string::murmur_hash(name);
			_resCtx->lookup[hash] = ri;
			return ri.id;
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

		static RID createShader(const char* name, const ShaderDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			Shader* s = new Shader;
			int idx = _resCtx->resources.size();
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
			IdString hash = string::murmur_hash(name);
			ri.index = idx;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::SHADER;
			_resCtx->lookup[hash] = ri;
			return ri.id;
		}

		// ------------------------------------------------------
		// parse constant buffer
		// ------------------------------------------------------
		void parseConstantBuffer(JSONReader& reader, int childIndex) {
			ConstantBufferDescriptor descriptor;
			reader.get(childIndex, "id", &descriptor.id);
			reader.get(childIndex, "size", &descriptor.size);
			const char* name = reader.get_string(childIndex, "name");
			createConstantBuffer(name, descriptor);
		}

		// ------------------------------------------------------
		// parse quad index buffer
		// ------------------------------------------------------
		void parseQuadIndexBuffer(JSONReader& reader, int childIndex) {
			QuadIndexBufferDescriptor descriptor;
			reader.get(childIndex, "id", &descriptor.id);
			reader.get(childIndex, "size", &descriptor.size);
			const char* name = reader.get_string(childIndex, "name");
			createQuadIndexBuffer(name, descriptor);
		}

		// ------------------------------------------------------
		// parse quad index buffer
		// ------------------------------------------------------
		void parseIndexBuffer(JSONReader& reader, int childIndex) {
			IndexBufferDescriptor descriptor;
			reader.get(childIndex, "id", &descriptor.id);
			reader.get(childIndex, "size", &descriptor.size);
			const char* name = reader.get_string(childIndex, "name");
			createIndexBuffer(name, descriptor);
		}

		// ------------------------------------------------------
		// parse vertex buffer
		// ------------------------------------------------------
		void parseVertexBuffer(JSONReader& reader, int childIndex) {
			VertexBufferDescriptor descriptor;
			reader.get(childIndex, "id", &descriptor.id);
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
			reader.get(childIndex, "id", &descriptor.id);
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
			reader.get(childIndex, "id", &descriptor.id);
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
			reader.get(childIndex, "id", &descriptor.id);
			descriptor.name = reader.get_string(childIndex, "file");
			const char* name = reader.get_string(childIndex, "name");
			loadTexture(name, descriptor);
		}

		// ------------------------------------------------------
		// parse texture cube
		// ------------------------------------------------------
		void parseTextureCube(JSONReader& reader, int childIndex) {
			TextureDescriptor descriptor;
			reader.get(childIndex, "id", &descriptor.id);
			descriptor.name = reader.get_string(childIndex, "file");
			const char* name = reader.get_string(childIndex, "name");
			loadTextureCube(name, descriptor);
		}

		// ------------------------------------------------------
		// parse scene
		// ------------------------------------------------------
		void parseScene(JSONReader& reader, int childIndex) {
			SceneDescriptor descriptor;
			reader.get(childIndex, "id", &descriptor.id);
			reader.get(childIndex, "size", &descriptor.size);
			descriptor.meshBuffer = reader.get_string(childIndex, "mesh_buffer");
			descriptor.camera = reader.get_string(childIndex, "camera");
			reader.get(childIndex, "depth_enabled", &descriptor.depthEnabled);
			const char* name = reader.get_string(childIndex, "name");
			createScene(name, descriptor);
		}

		// ------------------------------------------------------
		// parse camera
		// ------------------------------------------------------
		void parseCamera(JSONReader& reader, int childIndex) {
			CameraDescriptor descriptor;
			reader.get(childIndex, "id", &descriptor.id);
			reader.get(childIndex, "position", &descriptor.position);
			reader.get(childIndex, "target", &descriptor.target);
			descriptor.type= reader.get_string(childIndex, "type");
			const char* name = reader.get_string(childIndex, "name");
			createCamera(name, descriptor);
		}

		// ------------------------------------------------------
		// parse input layout
		// ------------------------------------------------------
		void parseInputLayout(JSONReader& reader, int childIndex) {
			InputLayoutDescriptor descriptor;
			descriptor.num = 0;
			reader.get(childIndex, "id", &descriptor.id);
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
			reader.get(childIndex, "id", &descriptor.id);
			descriptor.name = reader.get_string(childIndex, "file");
			const char* name = reader.get_string(childIndex, "name");
			loadFont(name, descriptor);
		}
		/*
		else if (reader.matches(children[i], "particles")) {
		ParticleSystemsDescriptor descriptor;
		reader.get(children[i], "id", &descriptor.id);
		reader.get(children[i], "sprite_buffer", &descriptor.spriteBuffer);
		const char* name = reader.get_string(children[i], "name");
		createParticleManager(descriptor);
		}
		*/

		// ------------------------------------------------------
		// parse sampler state
		// ------------------------------------------------------
		void parseSamplerState(JSONReader& reader, int childIndex) {
			SamplerStateDescriptor descriptor;
			reader.get(childIndex, "id", &descriptor.id);
			const char* mode = reader.get_string(childIndex, "addressU");
			int idx = findTextureAddressMode(mode);
			assert(idx >= 0);
			descriptor.addressU = idx;
			mode = reader.get_string(childIndex, "addressV");
			idx = findTextureAddressMode(mode);
			assert(idx >= 0);
			descriptor.addressV = idx;
			mode = reader.get_string(childIndex, "addressW");
			idx = findTextureAddressMode(mode);
			assert(idx >= 0);
			descriptor.addressW = idx;
			const char* name = reader.get_string(childIndex, "name");
			createSamplerState(name, descriptor);
		}

		// ------------------------------------------------------
		// parse sprite buffer
		// ------------------------------------------------------
		void parseSpriteBuffer(JSONReader& reader, int childIndex) {
			SpriteBufferDescriptor descriptor;
			reader.get(childIndex, "id", &descriptor.id);
			reader.get(childIndex, "size", &descriptor.size);
			reader.get(childIndex, "index_buffer", &descriptor.indexBuffer);
			const char* constantBufferName = reader.get_string(childIndex, "constant_buffer");
			descriptor.constantBuffer = find(constantBufferName, ResourceType::CONSTANTBUFFER);
			const char* vertexBufferName = reader.get_string(childIndex, "vertex_buffer");
			descriptor.vertexBuffer = find(vertexBufferName, ResourceType::VERTEXBUFFER);
			reader.get(childIndex, "shader", &descriptor.shader);
			reader.get(childIndex, "blend_state", &descriptor.blendstate);
			reader.get(childIndex, "color_map", &descriptor.colormap);
			const char* inputLayoutName = reader.get_string(childIndex, "input_layout");
			descriptor.inputlayout = find(inputLayoutName, ResourceType::INPUTLAYOUT);
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
			reader.get(childIndex, "id", &descriptor.id);
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
			reader.get(childIndex, "id", &descriptor.id);
			reader.get(childIndex, "size", &descriptor.size);
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
			descriptor.inputlayout= find(inputLayoutName, ResourceType::INPUTLAYOUT);
			const char* name = reader.get_string(childIndex, "name");
			createMeshBuffer(name, descriptor);
		}

		// ------------------------------------------------------
		// parse Mesh buffer
		// ------------------------------------------------------
		void parseSkyBox(JSONReader& reader, int childIndex) {
			SkyBoxDescriptor descriptor;
			reader.get(childIndex, "id", &descriptor.id);
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
		// parse Mesh
		// ------------------------------------------------------
		void parseMesh(JSONReader& reader, int childIndex) {
			MeshDescriptor descriptor;
			reader.get(childIndex, "id", &descriptor.id);
			reader.get(childIndex, "position", &descriptor.position);
			reader.get(childIndex, "scale", &descriptor.scale);
			reader.get(childIndex, "rotation", &descriptor.rotation);
			const char* name = reader.get_string(childIndex, "name");
			descriptor.fileName = name;
			createMesh(name, descriptor);
		}

		// ------------------------------------------------------
		// parse world
		// ------------------------------------------------------
		void parseWorld(JSONReader& reader, int childIndex) {
			WorldDescriptor descriptor;
			reader.get(childIndex, "id", &descriptor.id);
			reader.get(childIndex, "sprite_buffer", &descriptor.spriteBuffer);
			const char* name = reader.get_string(childIndex, "name");
			createWorld(name, descriptor);
		}

		// ------------------------------------------------------
		// parse IMGUI
		// ------------------------------------------------------
		void parseIMGUI(JSONReader& reader, int childIndex) {
			IMGUIDescriptor descriptor;
			reader.get(childIndex, "id", &descriptor.id);
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
			reader.get(childIndex, "id", &descriptor.id);
			reader.get(childIndex, "sprite_buffer", &descriptor.spriteBuffer);
			reader.get(childIndex, "font", &descriptor.font);
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
			for (uint32_t i = 0; i < MAX_RESOURCES; ++i) {
				ResourceIndex& index = _resCtx->resourceTable[i];
				index.id = INVALID_RID;
				index.index = 0;
				index.nameIndex = -1;
				index.type = ResourceType::UNKNOWN;
			}
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
			_resCtx->parsers[string::murmur_hash("camera")] = parseCamera;
			_resCtx->parsers[string::murmur_hash("texture_cube")] = parseTextureCube;
			_resCtx->parsers[string::murmur_hash("skybox")] = parseSkyBox;
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
			assert(ret);
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
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == type);
			return res_idx.index;
		}

		ID3D11Buffer* getIndexBuffer(RID rid) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == ResourceType::INDEXBUFFER);
			IndexBufferResource* res = static_cast<IndexBufferResource*>(_resCtx->resources[res_idx.index]);
			return res->get();
		}

		ID3D11BlendState* getBlendState(RID rid) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == ResourceType::BLENDSTATE);
			BlendStateResource* res = static_cast<BlendStateResource*>(_resCtx->resources[res_idx.index]);
			return res->get();
		}

		bool contains(RID rid, ResourceType type) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			return res_idx.type == type;
		}

		RID find(const char* name, ResourceType type) {
			IdString hash = string::murmur_hash(name);
			assert(_resCtx->lookup.find(hash) != _resCtx->lookup.end());
			const ResourceIndex& res_idx = _resCtx->lookup[hash];
			assert(res_idx.type == type);
			return res_idx.id;
		}

		ID3D11Buffer* getConstantBuffer(const char* name) {
			IdString hash = string::murmur_hash(name);
			if (_resCtx->lookup.find(hash) != _resCtx->lookup.end()) {
				const ResourceIndex& res_idx = _resCtx->lookup[hash];
				assert(res_idx.type == ResourceType::CONSTANTBUFFER);
				ConstantBufferResource* res = static_cast<ConstantBufferResource*>(_resCtx->resources[res_idx.index]);
				return res->get();
			}
			return 0;
		}

		ID3D11Buffer* getConstantBuffer(RID rid) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == ResourceType::CONSTANTBUFFER);
			ConstantBufferResource* res = static_cast<ConstantBufferResource*>(_resCtx->resources[res_idx.index]);
			return res->get();
		}

		ID3D11Buffer* getVertexBuffer(RID rid) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == ResourceType::VERTEXBUFFER);
			VertexBufferResource* res = static_cast<VertexBufferResource*>(_resCtx->resources[res_idx.index]);
			return res->get();
		}

		ID3D11InputLayout* getInputLayout(const char* name) {
			IdString hash = string::murmur_hash(name);
			if (_resCtx->lookup.find(hash) != _resCtx->lookup.end()) {
				const ResourceIndex& res_idx = _resCtx->lookup[hash];
				assert(res_idx.type == ResourceType::INPUTLAYOUT);
				InputLayoutResource* res = static_cast<InputLayoutResource*>(_resCtx->resources[res_idx.index]);
				return res->get();
			}
			return 0;
		}

		ID3D11InputLayout* getInputLayout(RID rid) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == ResourceType::INPUTLAYOUT);
			InputLayoutResource* res = static_cast<InputLayoutResource*>(_resCtx->resources[res_idx.index]);
			return res->get();
		}

		ID3D11ShaderResourceView* getShaderResourceView(RID rid) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == ResourceType::TEXTURE);
			ShaderResourceViewResource* res = static_cast<ShaderResourceViewResource*>(_resCtx->resources[res_idx.index]);
			return res->get();
		}

		ID3D11SamplerState* getSamplerState(RID rid) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == ResourceType::SAMPLERSTATE);
			SamplerStateResource* res = static_cast<SamplerStateResource*>(_resCtx->resources[res_idx.index]);
			return res->get();
		}

		Shader* getShader(RID rid) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == ResourceType::SHADER);
			ShaderResource* res = static_cast<ShaderResource*>(_resCtx->resources[res_idx.index]);
			return res->get();
		}

		Bitmapfont* getFont(RID rid) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == ResourceType::BITMAPFONT);
			BitmapfontResource* res = static_cast<BitmapfontResource*>(_resCtx->resources[res_idx.index]);
			return res->get();
		}

		SpriteBuffer* getSpriteBuffer(const char* name) {
			IdString hash = string::murmur_hash(name);
			assert(_resCtx->lookup.find(hash) != _resCtx->lookup.end());
			const ResourceIndex& res_idx = _resCtx->lookup[hash];
			assert(res_idx.type == ResourceType::SPRITEBUFFER);
			SpriteBufferResource* res = static_cast<SpriteBufferResource*>(_resCtx->resources[res_idx.index]);
			return res->get();			
		}

		SpriteBuffer* getSpriteBuffer(RID rid) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == ResourceType::SPRITEBUFFER);
			SpriteBufferResource* res = static_cast<SpriteBufferResource*>(_resCtx->resources[res_idx.index]);
			return res->get();
		}

		World* getWorld(RID rid) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == ResourceType::WORLD);
			WorldResource* res = static_cast<WorldResource*>(_resCtx->resources[res_idx.index]);
			return res->get();
		}

		Mesh* getMesh(RID rid) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == ResourceType::MESH);
			MeshResource* res = static_cast<MeshResource*>(_resCtx->resources[res_idx.index]);
			return res->get();
		}

		Mesh* getMesh(const char* name) {
			IdString hash = string::murmur_hash(name);
			assert(_resCtx->lookup.find(hash) != _resCtx->lookup.end());
			const ResourceIndex& res_idx = _resCtx->lookup[hash];
			assert(res_idx.type == ResourceType::MESH);
			MeshResource* res = static_cast<MeshResource*>(_resCtx->resources[res_idx.index]);
			return res->get();
		}

		SkyBox* getSkyBox(const char* name) {
			IdString hash = string::murmur_hash(name);
			assert(_resCtx->lookup.find(hash) != _resCtx->lookup.end());
			const ResourceIndex& res_idx = _resCtx->lookup[hash];
			assert(res_idx.type == ResourceType::SKYBOX);
			SkyBoxResource* res = static_cast<SkyBoxResource*>(_resCtx->resources[res_idx.index]);
			return res->get();
		}

		MeshBuffer* getMeshBuffer(RID rid) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == ResourceType::MESHBUFFER);
			MeshBufferResource* res = static_cast<MeshBufferResource*>(_resCtx->resources[res_idx.index]);
			return res->get();
		}

		MeshBuffer* getMeshBuffer(const char* name) {
			IdString hash = string::murmur_hash(name);
			assert(_resCtx->lookup.find(hash) != _resCtx->lookup.end());
			const ResourceIndex& res_idx = _resCtx->lookup[hash];
			assert(res_idx.type == ResourceType::MESHBUFFER);
			MeshBufferResource* res = static_cast<MeshBufferResource*>(_resCtx->resources[res_idx.index]);
			return res->get();
		}

		Scene* getScene(const char* name) {
			IdString hash = string::murmur_hash(name);
			assert(_resCtx->lookup.find(hash) != _resCtx->lookup.end());
			const ResourceIndex& res_idx = _resCtx->lookup[hash];
			assert(res_idx.type == ResourceType::SCENE);
			SceneResource* res = static_cast<SceneResource*>(_resCtx->resources[res_idx.index]);
			return res->get();
		}

		Camera* getCamera(const char* name) {
			IdString hash = string::murmur_hash(name);
			assert(_resCtx->lookup.find(hash) != _resCtx->lookup.end());
			const ResourceIndex& res_idx = _resCtx->lookup[hash];
			assert(res_idx.type == ResourceType::CAMERA);
			CameraResource* res = static_cast<CameraResource*>(_resCtx->resources[res_idx.index]);
			return res->get();
		}

		QuadBuffer* getQuadBuffer(RID rid) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == ResourceType::QUADBUFFER);
			QuadBufferResource* res = static_cast<QuadBufferResource*>(_resCtx->resources[res_idx.index]);
			return res->get();
		}

		GUIDialog* getGUIDialog(RID rid) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == ResourceType::GUIDIALOG);
			GUIDialogResource* res = static_cast<GUIDialogResource*>(_resCtx->resources[res_idx.index]);
			return res->get();
		}

		ParticleManager* getParticleManager() {
			return _resCtx->particles;
		}

		void debug() {
			for (uint32_t i = 0; i < MAX_RESOURCES; ++i) {
				ResourceIndex& index = _resCtx->resourceTable[i];
				if (index.type != ResourceType::UNKNOWN) {
					if (index.nameIndex != -1) {
						const char* text = _resCtx->nameBuffer.data + index.nameIndex;
						LOG << index.id << " Type: " << ResourceTypeNames[index.type] << " name: " << text;
					}
				}
			}
		}

		void save(const ReportWriter& writer) {
			writer.startBox("Resources");
			const char* HEADERS[] = { "ID", "Index", "Type", "Name" };
			writer.startTable(HEADERS, 4);
			for (uint32_t i = 0; i < MAX_RESOURCES; ++i) {
				ResourceIndex& index = _resCtx->resourceTable[i];
				if (index.type != ResourceType::UNKNOWN) {
					if (index.nameIndex != -1) {
						writer.startRow();
						writer.addCell(index.id);
						writer.addCell(index.index);
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