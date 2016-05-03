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

namespace ds {

	namespace res {

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
			uint32_t semanticIndex;
			DXGI_FORMAT format;
			uint32_t size;

			InputElementDescriptor(const char* sem, uint32_t index, DXGI_FORMAT f, uint32_t s) :
				semantic(sem), semanticIndex(index), format(f), size(s) {
			}
		};

		static const InputElementDescriptor INPUT_ELEMENT_DESCRIPTIONS[] = {

			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 12 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 12 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 8 },
			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 8 },
			{ "TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT, 8 },
			{ "TEXCOORD", 3, DXGI_FORMAT_R32G32_FLOAT, 8 },
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
			"UNKNOWN"
		};

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
			std::vector<ID3D11Buffer*> indexBuffers;
			std::vector<ID3D11Buffer*> constantBuffers;
			std::vector<ID3D11ShaderResourceView*> shaderResourceViews;
			std::vector<ID3D11BlendState*> blendStates;
			std::vector<ID3D11Buffer*> vertexBuffers;
			std::vector<Shader*> shaders;
			std::vector<ID3D11InputLayout*> layouts;
			std::vector<Bitmapfont*> fonts;
			std::vector<SpriteBuffer*> spriteBuffers;
			std::vector<QuadBuffer*> quadBuffers;
			std::vector<World*> worlds;
			std::vector<ID3D11SamplerState*> samplerStates;
			std::vector<GUIDialog*> dialogs;
			uint32_t resourceIndex;
			ResourceIndex resourceTable[MAX_RESOURCES];
			ParticleManager* particles;
		};

		static ResourceContext* _resCtx;

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
		}

		// ------------------------------------------------------
		// shutdown
		// ------------------------------------------------------
		void shutdown() {
			for (size_t i = 0; i < _resCtx->indexBuffers.size(); ++i) {
				_resCtx->indexBuffers[i]->Release();
			}
			for (size_t i = 0; i < _resCtx->blendStates.size(); ++i) {
				_resCtx->blendStates[i]->Release();
			}
			for (size_t i = 0; i < _resCtx->layouts.size(); ++i) {
				_resCtx->layouts[i]->Release();
			}
			for (size_t i = 0; i < _resCtx->shaders.size(); ++i) {
				Shader* shader = _resCtx->shaders[i];
				if (shader->vertexShader != 0) {
					shader->vertexShader->Release();
				}
				if (shader->pixelShader != 0) {
					shader->pixelShader->Release();
				}
				if (shader->vertexShaderBuffer != 0) {
					shader->vertexShaderBuffer->Release();
				}
				delete _resCtx->shaders[i];
			}
			for (size_t i = 0; i < _resCtx->shaderResourceViews.size(); ++i) {
				_resCtx->shaderResourceViews[i]->Release();
			}
			for (size_t i = 0; i < _resCtx->vertexBuffers.size(); ++i) {
				_resCtx->vertexBuffers[i]->Release();
			}
			for (size_t i = 0; i < _resCtx->constantBuffers.size(); ++i) {
				_resCtx->constantBuffers[i]->Release();
			}
			for (size_t i = 0; i < _resCtx->fonts.size(); ++i) {
				delete _resCtx->fonts[i];
			}
			for (size_t i = 0; i < _resCtx->spriteBuffers.size(); ++i) {
				delete _resCtx->spriteBuffers[i];
			}
			for (size_t i = 0; i < _resCtx->worlds.size(); ++i) {
				delete _resCtx->worlds[i];
			}
			for (size_t i = 0; i < _resCtx->samplerStates.size(); ++i) {
				_resCtx->samplerStates[i]->Release();
			}
			for (size_t i = 0; i < _resCtx->dialogs.size(); ++i) {
				delete _resCtx->dialogs[i];
			}
			for (size_t i = 0; i < _resCtx->quadBuffers.size(); ++i) {
				delete _resCtx->quadBuffers[i];
			}
			if (_resCtx->particles != 0) {
				delete _resCtx->particles;
			}
			gui::shutdown();
			delete _resCtx;
		}



		// ------------------------------------------------------
		// cretate quad index buffer
		// ------------------------------------------------------
		static RID createQuadIndexBuffer(const char* name,const QuadIndexBufferDescriptor& descriptor) {
			int idx = _resCtx->indexBuffers.size();
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
			_resCtx->indexBuffers.push_back(buffer);
			ri.index = idx;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::INDEXBUFFER;
			return ri.id;
		}

		// ------------------------------------------------------
		// create index buffer
		// ------------------------------------------------------
		static RID createIndexBuffer(const char* name, const IndexBufferDescriptor& descriptor) {
			int idx = _resCtx->indexBuffers.size();
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
			_resCtx->indexBuffers.push_back(buffer);
			ri.index = idx;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::INDEXBUFFER;
			return ri.id;
		}

		// ------------------------------------------------------
		// create constant buffer
		// ------------------------------------------------------
		static RID createConstantBuffer(const char* name, const ConstantBufferDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			int index = _resCtx->constantBuffers.size();
			D3D11_BUFFER_DESC constDesc;
			ZeroMemory(&constDesc, sizeof(constDesc));
			constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			constDesc.ByteWidth = descriptor.size;
			constDesc.Usage = D3D11_USAGE_DEFAULT;
			ID3D11Buffer* buffer = 0;
			HRESULT d3dResult = _resCtx->device->CreateBuffer(&constDesc, 0, &buffer);
			if (FAILED(d3dResult))	{
				DXTRACE_MSG("Failed to create constant buffer!");
				return -1;
			}
			_resCtx->constantBuffers.push_back(buffer);
			ri.index = index;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::CONSTANTBUFFER;
			return ri.id;
		}

		// ------------------------------------------------------
		// create sprite buffer
		// ------------------------------------------------------
		static RID createSpriteBuffer(const char* name, const SpriteBufferDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			int index = _resCtx->spriteBuffers.size();
			SpriteBuffer* buffer = new SpriteBuffer(descriptor);
			_resCtx->spriteBuffers.push_back(buffer);
			ri.index = index;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::SPRITEBUFFER;
			return ri.id;
		}

		// ------------------------------------------------------
		// create quad buffer
		// ------------------------------------------------------
		static RID createQuadBuffer(const char* name, const QuadBufferDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			int index = _resCtx->quadBuffers.size();
			QuadBuffer* buffer = new QuadBuffer(descriptor);
			_resCtx->quadBuffers.push_back(buffer);
			ri.index = index;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::QUADBUFFER;
			return ri.id;
		}

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
		// create sampler state
		// ------------------------------------------------------
		RID createSamplerState(const char* name, const SamplerStateDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			int index = _resCtx->samplerStates.size();
			D3D11_SAMPLER_DESC colorMapDesc;
			ZeroMemory(&colorMapDesc, sizeof(colorMapDesc));
			colorMapDesc.AddressU = TEXTURE_ADDRESS_MODES[descriptor.addressU].mode;
			colorMapDesc.AddressV = TEXTURE_ADDRESS_MODES[descriptor.addressV].mode;
			colorMapDesc.AddressW = TEXTURE_ADDRESS_MODES[descriptor.addressW].mode;
			colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;
			ID3D11SamplerState* sampler;
			HRESULT d3dResult = _resCtx->device->CreateSamplerState(&colorMapDesc, &sampler);
			if (FAILED(d3dResult)) {
				DXTRACE_MSG("Failed to create SamplerState!");
				return INVALID_RID;
			}
			_resCtx->samplerStates.push_back(sampler);
			ri.index = index;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::SAMPLERSTATE;
			return ri.id;
		}

		// ------------------------------------------------------
		// load texture
		// ------------------------------------------------------
		static RID loadTexture(const char* name, const TextureDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			int idx = _resCtx->shaderResourceViews.size();
			ID3D11ShaderResourceView* srv = 0;
			char buffer[256];
			sprintf_s(buffer, 256, "content\\textures\\%s", descriptor.name);
			HRESULT d3dResult = D3DX11CreateShaderResourceViewFromFile(_resCtx->device, buffer, 0, 0, &srv, 0);
			if (FAILED(d3dResult)) {
				DXTRACE_MSG("Failed to load the texture image!");
				return INVALID_RID;
			}
			_resCtx->shaderResourceViews.push_back(srv);
			ri.index = idx;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::TEXTURE;
			return ri.id;
		}

		// ------------------------------------------------------
		// load bitmap font
		// ------------------------------------------------------
		static RID loadFont(const char* name,const BitmapfontDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			int idx = _resCtx->fonts.size();

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
			_resCtx->fonts.push_back(font);
			ri.index = idx;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::BITMAPFONT;
			return ri.id;
		}

		// ------------------------------------------------------
		// create blend state
		// ------------------------------------------------------
		static RID createBlendState(const char* name, const BlendStateDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			int idx = _resCtx->blendStates.size();
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
			_resCtx->blendStates.push_back(state);
			ri.index = idx;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::BLENDSTATE;
			return ri.id;
		}

		// ------------------------------------------------------
		// create world
		// ------------------------------------------------------
		static RID createWorld(const char* name, const WorldDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			World* w = new World(descriptor);
			int idx = _resCtx->worlds.size();
			_resCtx->worlds.push_back(w);
			ri.index = idx;
			ri.id = descriptor.id;
			ri.type = ResourceType::WORLD;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
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
			int idx = _resCtx->dialogs.size();
			_resCtx->dialogs.push_back(dialog);
			ri.index = idx;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::GUIDIALOG;
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
			bufferDesciption.ByteWidth = descriptor.size;

			ID3D11Buffer* buffer = 0;
			HRESULT d3dResult = _resCtx->device->CreateBuffer(&bufferDesciption, 0, &buffer);
			if (FAILED(d3dResult))	{
				DXTRACE_MSG("Failed to create buffer!");
				return -1;
			}
			int idx = _resCtx->vertexBuffers.size();
			_resCtx->vertexBuffers.push_back(buffer);
			ri.index = idx;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::VERTEXBUFFER;
			return ri.id;
		}

		// ------------------------------------------------------
		// create vertex buffer
		// ------------------------------------------------------
		static RID createInputLayout(const char* name, const InputLayoutDescriptor& descriptor) {
			ResourceIndex& ri = _resCtx->resourceTable[descriptor.id];
			assert(ri.type == ResourceType::UNKNOWN);
			int idx = _resCtx->layouts.size();
			D3D11_INPUT_ELEMENT_DESC* descriptors = new D3D11_INPUT_ELEMENT_DESC[descriptor.num];
			uint32_t index = 0;
			uint32_t counter = 0;
			for (int i = 0; i < descriptor.num; ++i) {
				const InputElementDescriptor& d = INPUT_ELEMENT_DESCRIPTIONS[descriptor.indices[i]];
				D3D11_INPUT_ELEMENT_DESC& descriptor = descriptors[counter++];
				descriptor.SemanticName = d.semantic;
				descriptor.SemanticIndex = d.semanticIndex;
				descriptor.Format = d.format;
				descriptor.InputSlot = 0;
				descriptor.AlignedByteOffset = index;
				descriptor.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				descriptor.InstanceDataStepRate = 0;
				index += d.size;
			}
			ID3D11InputLayout* layout = 0;
			const ResourceIndex& res_idx = _resCtx->resourceTable[descriptor.shader];
			assert(res_idx.type == ResourceType::SHADER);
			Shader* s = _resCtx->shaders[res_idx.index];
			HRESULT d3dResult = _resCtx->device->CreateInputLayout(descriptors, descriptor.num, s->vertexShaderBuffer->GetBufferPointer(), s->vertexShaderBuffer->GetBufferSize(), &layout);
			if (d3dResult < 0) {
				return INVALID_RID;
			}
			delete[] descriptors;
			_resCtx->layouts.push_back(layout);
			ri.index = idx;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::INPUTLAYOUT;
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
					OutputDebugStringA((char*)errorBuffer->GetBufferPointer());
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
			int idx = _resCtx->shaders.size();
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
			s->samplerState = getSamplerState(descriptor.samplerState);
			_resCtx->shaders.push_back(s);
			ri.index = idx;
			ri.id = descriptor.id;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = ResourceType::SHADER;
			return ri.id;
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
			for (int i = 0; i < 6; ++i) {
				if (strcmp(INPUT_ELEMENT_DESCRIPTIONS[i].semantic, name) == 0) {
					return i;
				}
			}
			return -1;
		}



		void parseJSONFile() {
			JSONReader reader;
			bool ret = reader.parse("content\\resources.json");
			assert(ret);
			int children[256];
			int num = reader.get_categories(children, 256);
			for (int i = 0; i < num; ++i) {
				if (reader.matches(children[i], "quad_index_buffer")) {
					QuadIndexBufferDescriptor descriptor;
					reader.get(children[i], "id", &descriptor.id);
					reader.get(children[i], "size", &descriptor.size);
					const char* name = reader.get_string(children[i], "name");
					createQuadIndexBuffer(name,descriptor);
				}
				else if (reader.matches(children[i], "constant_buffer")) {
					ConstantBufferDescriptor descriptor;
					reader.get(children[i], "id", &descriptor.id);
					reader.get(children[i], "size", &descriptor.size);
					const char* name = reader.get_string(children[i], "name");
					createConstantBuffer(name,descriptor);
				}
				else if (reader.matches(children[i], "vertex_buffer")) {
					VertexBufferDescriptor descriptor;
					reader.get(children[i], "id", &descriptor.id);
					reader.get(children[i], "size", &descriptor.size);
					reader.get(children[i], "dynamic", &descriptor.dynamic);
					reader.get(children[i], "layout", &descriptor.layout);
					const char* name = reader.get_string(children[i], "name");
					createVertexBuffer(name, descriptor);
				}
				else if (reader.matches(children[i], "shader")) {
					ShaderDescriptor descriptor;
					reader.get(children[i], "id", &descriptor.id);
					descriptor.file = reader.get_string(children[i], "file");
					descriptor.vertexShader = reader.get_string(children[i], "vertex_shader");
					descriptor.pixelShader = reader.get_string(children[i], "pixel_shader");
					descriptor.model = reader.get_string(children[i], "shader_model");
					reader.get(children[i], "sampler_state", &descriptor.samplerState);
					const char* name = reader.get_string(children[i], "name");
					createShader(name, descriptor);
				}
				else if (reader.matches(children[i], "blendstate")) {
					// FIXME: assert that every entry is != -1
					BlendStateDescriptor descriptor;
					reader.get(children[i], "id", &descriptor.id);
					const char* entry = reader.get_string(children[i], "src_blend");
					descriptor.srcBlend = findBlendState(entry);
					entry = reader.get_string(children[i], "dest_blend");
					descriptor.destBlend = findBlendState(entry);
					entry = reader.get_string(children[i], "src_blend_alpha");
					descriptor.srcAlphaBlend = findBlendState(entry);
					entry = reader.get_string(children[i], "dest_blend_alpha");
					descriptor.destAlphaBlend = findBlendState(entry);
					reader.get(children[i], "alpha_enabled", &descriptor.alphaEnabled);
					const char* name = reader.get_string(children[i], "name");
					createBlendState(name, descriptor);
				}
				else if (reader.matches(children[i], "texture")) {
					TextureDescriptor descriptor;
					reader.get(children[i], "id", &descriptor.id);
					descriptor.name = reader.get_string(children[i], "file");
					const char* name = reader.get_string(children[i], "name");
					loadTexture(name, descriptor);
				}
				else if (reader.matches(children[i], "input_layout")) {
					InputLayoutDescriptor descriptor;
					descriptor.num = 0;
					reader.get(children[i], "id", &descriptor.id);
					const char* attributes = reader.get_string(children[i], "attributes");
					char buffer[256];
					sprintf_s(buffer, 256, "%s", attributes);
					char *token = token = strtok(buffer, ",");
					while (token) {
						int element = findInputElement(token);
						descriptor.indices[descriptor.num++] = element;
						token = strtok(NULL, ",");
					}
					reader.get(children[i], "shader", &descriptor.shader);
					const char* name = reader.get_string(children[i], "name");
					createInputLayout(name, descriptor);
				}
				else if (reader.matches(children[i], "font")) {
					BitmapfontDescriptor descriptor;
					reader.get(children[i], "id", &descriptor.id);
					descriptor.name = reader.get_string(children[i], "file");
					const char* name = reader.get_string(children[i], "name");
					loadFont(name,descriptor);
				}
				else if (reader.matches(children[i], "particles")) {
					ParticleSystemsDescriptor descriptor;
					reader.get(children[i], "id", &descriptor.id);
					reader.get(children[i], "sprite_buffer", &descriptor.spriteBuffer);
					const char* name = reader.get_string(children[i], "name");
					createParticleManager(descriptor);
				}
				else if (reader.matches(children[i], "sampler_state")) {
					SamplerStateDescriptor descriptor;
					reader.get(children[i], "id", &descriptor.id);
					const char* mode = reader.get_string(children[i], "addressU");
					int idx = findTextureAddressMode(mode);
					assert(idx >= 0);
					descriptor.addressU = idx;
					mode = reader.get_string(children[i], "addressV");
					idx = findTextureAddressMode(mode);
					assert(idx >= 0);
					descriptor.addressV = idx;
					mode = reader.get_string(children[i], "addressW");
					idx = findTextureAddressMode(mode);
					assert(idx >= 0);
					descriptor.addressW = idx;
					const char* name = reader.get_string(children[i], "name");
					createSamplerState(name, descriptor);
				}
				else if (reader.matches(children[i], "sprite_buffer")) {
					SpriteBufferDescriptor descriptor;
					reader.get(children[i], "id", &descriptor.id);
					reader.get(children[i], "size", &descriptor.size);
					reader.get(children[i], "index_buffer", &descriptor.indexBuffer);
					reader.get(children[i], "constant_buffer", &descriptor.constantBuffer);
					reader.get(children[i], "vertex_buffer", &descriptor.vertexBuffer);
					reader.get(children[i], "shader", &descriptor.shader);
					reader.get(children[i], "blend_state", &descriptor.blendstate);
					reader.get(children[i], "color_map", &descriptor.colormap);
					reader.get(children[i], "input_layout", &descriptor.inputlayout);
					if (reader.contains_property(children[i], "font")) {
						reader.get(children[i], "font", &descriptor.font);
					}
					else {
						descriptor.font = INVALID_RID;
					}
					const char* name = reader.get_string(children[i], "name");
					createSpriteBuffer(name, descriptor);
				}
				else if (reader.matches(children[i], "quad_buffer")) {
					QuadBufferDescriptor descriptor;
					reader.get(children[i], "id", &descriptor.id);
					reader.get(children[i], "size", &descriptor.size);
					reader.get(children[i], "index_buffer", &descriptor.indexBuffer);
					reader.get(children[i], "constant_buffer", &descriptor.constantBuffer);
					reader.get(children[i], "vertex_buffer", &descriptor.vertexBuffer);
					reader.get(children[i], "shader", &descriptor.shader);
					reader.get(children[i], "blend_state", &descriptor.blendstate);
					reader.get(children[i], "color_map", &descriptor.colormap);
					reader.get(children[i], "input_layout", &descriptor.inputlayout);
					const char* name = reader.get_string(children[i], "name");
					createQuadBuffer(name, descriptor);
				}
				else if (reader.matches(children[i], "world")) {
					WorldDescriptor descriptor;
					reader.get(children[i], "id", &descriptor.id);
					reader.get(children[i], "sprite_buffer", &descriptor.spriteBuffer);
					const char* name = reader.get_string(children[i], "name");
					createWorld(name, descriptor);
				}
				else if (reader.matches(children[i], "imgui")) {
					IMGUIDescriptor descriptor;
					reader.get(children[i], "id", &descriptor.id);
					reader.get(children[i], "sprite_buffer", &descriptor.spriteBuffer);
					reader.get(children[i], "font", &descriptor.font);
					gui::initialize(descriptor);
				}
				else if (reader.matches(children[i], "dialog")) {
					GUIDialogDescriptor descriptor;
					reader.get(children[i], "id", &descriptor.id);
					reader.get(children[i], "sprite_buffer", &descriptor.spriteBuffer);
					reader.get(children[i], "font", &descriptor.font);
					descriptor.file = reader.get_string(children[i], "file");
					const char* name = reader.get_string(children[i], "name");
					createDialog(name, descriptor);
				}
				/*
				dialogs {
				id : 14
				sprite_buffer : 8
				font : 7
				}
				*/
			}
		}

		uint32_t getIndex(RID rid, ResourceType type) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == type);
			return res_idx.index;
		}

		ID3D11Buffer* getIndexBuffer(RID rid) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == ResourceType::INDEXBUFFER);
			return _resCtx->indexBuffers[res_idx.index];
		}

		ID3D11BlendState* getBlendState(RID rid) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == ResourceType::BLENDSTATE);
			return _resCtx->blendStates[res_idx.index];
		}

		ID3D11Buffer* getConstantBuffer(RID rid) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == ResourceType::CONSTANTBUFFER);
			return _resCtx->constantBuffers[res_idx.index];
		}

		ID3D11Buffer* getVertexBuffer(RID rid) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == ResourceType::VERTEXBUFFER);
			return _resCtx->vertexBuffers[res_idx.index];
		}

		ID3D11InputLayout* getInputLayout(RID rid) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == ResourceType::INPUTLAYOUT);
			return _resCtx->layouts[res_idx.index];
		}

		ID3D11ShaderResourceView* getShaderResourceView(RID rid) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == ResourceType::TEXTURE);
			return _resCtx->shaderResourceViews[res_idx.index];
		}

		ID3D11SamplerState* getSamplerState(RID rid) {
			uint32_t idx = getIndex(rid, ResourceType::SAMPLERSTATE);
			return _resCtx->samplerStates[idx];
		}

		Shader* getShader(RID rid) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == ResourceType::SHADER);
			return _resCtx->shaders[res_idx.index];
		}

		Bitmapfont* getFont(RID rid) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == ResourceType::BITMAPFONT);
			return _resCtx->fonts[res_idx.index];
		}

		SpriteBuffer* getSpriteBuffer(RID rid) {
			const ResourceIndex& res_idx = _resCtx->resourceTable[rid];
			assert(res_idx.type == ResourceType::SPRITEBUFFER);
			return _resCtx->spriteBuffers[res_idx.index];
		}

		World* getWorld(RID rid) {
			uint32_t idx = getIndex(rid, ResourceType::WORLD);
			return _resCtx->worlds[idx];
		}

		QuadBuffer* getQuadBuffer(RID rid) {
			uint32_t idx = getIndex(rid, ResourceType::QUADBUFFER);
			return _resCtx->quadBuffers[idx];
		}

		GUIDialog* getGUIDialog(RID rid) {
			uint32_t idx = getIndex(rid, ResourceType::GUIDIALOG);
			return _resCtx->dialogs[idx];
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