#include "ResourceContainer.h"
#include <D3D11.h>
#include <D3Dcompiler.h>
#include "core\io\json.h"
#include "ResourceDescriptors.h"
#include "..\renderer\graphics.h"
#include <string.h>
#include "core\log\Log.h"
#include "..\renderer\render_types.h"
#include "..\imgui\IMGUI.h"
#include "core\base\Assert.h"
#include "Resource.h"
#include "core\string\StaticHash.h"
#include "core\string\StringUtils.h"
#include "..\audio\AudioManager.h"
#define STB_IMAGE_IMPLEMENTATION
#include "..\renderer\stb_image.h"
#include "core\io\ReportWriter.h"
#include "parser\ResourceParser.h"
#include "parser\ConstantBufferParser.h"
#include "parser\DialogParser.h"
#include "parser\MaterialParser.h"
#include "parser\FontParser.h"
#include "parser\SoundParser.h"
#include "parser\RenderTargetParser.h"
#include "parser\SpritesheetParser.h"
#include "parser\TextureParser.h"
#include "parser\SamplerStateParser.h"
#include "parser\ParticleManagerParser.h"
#include "parser\QuadIndexBufferParser.h"
#include "parser\ScriptParser.h"
#include "parser\SquareBufferParser.h"
#include "parser\WorldEntityTemplatesParser.h"
#include "parser\IMGUIParser.h"
#include "parser\ShaderParser.h"
#include "parser\BlendStateParser.h"

namespace ds {

	namespace res {	

		
		static std::map<StaticHash, ParseFunc> parsers;

		static std::map<StaticHash, ResourceParser*> nparsers;
		

		static ResourceContext* _resCtx;

		

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

		

		// ------------------------------------------------------
		// find inputelement by name
		// ------------------------------------------------------
		static int findInputElement(const char* name) {
			for (int i = 0; i < 4; ++i) {
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
			StaticHash hash = StaticHash(name);
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
		// create square buffer
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
			if (descriptor.dataSize == 0) {
				HRESULT d3dResult = _resCtx->device->CreateBuffer(&bufferDesciption, 0, &buffer);
				if (FAILED(d3dResult))	{
					DXTRACE_MSG("Failed to create buffer!");
					return -1;
				}
			}
			else {
				D3D11_SUBRESOURCE_DATA resource;
				resource.pSysMem = descriptor.data;
				resource.SysMemPitch = 0;
				resource.SysMemSlicePitch = 0;
				HRESULT d3dResult = _resCtx->device->CreateBuffer(&bufferDesciption, &resource, &buffer);
				if (FAILED(d3dResult))	{
					DXTRACE_MSG("Failed to create buffer!");
					return -1;
				}
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

		
		RID createEmptyShader(const char* name) {
			Shader* s = new Shader;
			s->vertexShaderBuffer = 0;
			ShaderResource* cbr = new ShaderResource(s);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::SHADER);
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

		RID createBlendState(const char* name, const BlendStateDescriptor& descriptor) {
			BlendStateParser* p = (BlendStateParser*)nparsers[SID("blendstate")];
			return p->createBlendState(name, descriptor);
		}

		RID createConstantBuffer(const char* name, const ConstantBufferDescriptor& descriptor) {
			ConstantBufferParser* p = (ConstantBufferParser*)nparsers[SID("constant_buffer")];
			return p->createConsantBuffer(name, descriptor);
		}

		RID createMaterial(const char* name, const MaterialDescriptor& descriptor) {
			MaterialParser* p = (MaterialParser*)nparsers[SID("material")];
			return p->createMaterial(name, descriptor);
		}

		RID createSamplerState(const char* name, const SamplerStateDescriptor& descriptor) {
			SamplerStateParser* p = (SamplerStateParser*)nparsers[SID("sampler_state")];
			return p->createSamplerState(name, descriptor);
		}

		// ------------------------------------------------------
		// initialize
		// ------------------------------------------------------
		void initialize(ID3D11Device* device) {
			_resCtx = new ResourceContext;
			_resCtx->device = device;
			_resCtx->resourceIndex = 0;				
			_resCtx->particles = 0;
			nparsers[SID("constant_buffer")] = new ConstantBufferParser(_resCtx);
			nparsers[SID("dialog")] = new DialogParser(_resCtx);
			nparsers[SID("material")] = new MaterialParser(_resCtx);
			nparsers[SID("font")] = new FontParser(_resCtx);
			nparsers[SID("sound")] = new SoundParser(_resCtx);
			nparsers[SID("spritesheet")] = new SpritesheetParser(_resCtx);
			nparsers[SID("render_target")] = new RenderTargetParser(_resCtx);
			nparsers[SID("texture")] = new TextureParser(_resCtx);
			nparsers[SID("sampler_state")] = new SamplerStateParser(_resCtx);
			nparsers[SID("particle_manager")] = new ParticleManagerParser(_resCtx);
			nparsers[SID("quad_index_buffer")] = new QuadIndexBufferParser(_resCtx);
			nparsers[SID("script")] = new ScriptParser(_resCtx);
			nparsers[SID("square_buffer")] = new SquareBufferParser(_resCtx);
			nparsers[SID("world_entity_templates")] = new WorldEntityTemplatesParser(_resCtx);
			nparsers[SID("imgui")] = new IMGUIParser(_resCtx);
			nparsers[SID("shader")] = new ShaderParser(_resCtx);
			nparsers[SID("blendstate")] = new BlendStateParser(_resCtx);

			parsers[SID("index_buffer")] = parseIndexBuffer;
			parsers[SID("sprite_buffer")] = parseSpriteBuffer;
			parsers[SID("input_layout")] = parseInputLayout;
			parsers[SID("vertex_buffer")] = parseVertexBuffer;					
			parsers[SID("mesh")] = parseMesh;
			parsers[SID("mesh_buffer")] = parseMeshBuffer;
			parsers[SID("quad_buffer")] = parseQuadBuffer;
			parsers[SID("scene")] = parseScene;
			parsers[SID("texture_cube")] = parseTextureCube;
			parsers[SID("skybox")] = parseSkyBox;									
			
			
		}

		// ------------------------------------------------------
		// parse specific json file
		// ------------------------------------------------------
		void parseJSONFile(const char* fileName) {
			JSONReader reader;
			char buffer[256];
			StaticHash hh = SID("content\\resources.json");

			StaticHash importHash = SID("import");
			sprintf_s(buffer, 256, "content\\%s", fileName);
			LOG << "Loading resource file: " << fileName;
			bool ret = reader.parse(SID(buffer));
			XASSERT(ret, "Failed parsing json file");
			int children[256];
			int num = reader.get_categories(children, 256);
			for (int i = 0; i < num; ++i) {
				StaticHash hash = SID(reader.get_category_name(i));
				// special category to import other files
				if (hash == importHash) {
					const char* fileName = reader.get_string(children[i], "file");
					parseJSONFile(fileName);
				}
				else {
					if (parsers.find(hash) != parsers.end()) {
						ParseFunc f = parsers[hash];
						LOG << "Parsing '" << reader.get_category_name(i) << "'";
						(*f)(reader, i);
					}
					else if (nparsers.find(hash) != nparsers.end()) {
						ResourceParser* f = nparsers[hash];
						LOG << "Parsing '" << reader.get_category_name(i) << "'";
						f->parse(reader, i);
					}
					else {
						LOGE << "No matching parser for '" << reader.get_category_name(i) << "'";
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

		bool contains(StaticHash hash, ResourceType type) {
			int idx = -1;
			for (uint32_t i = 0; i < _resCtx->indices.size(); ++i) {
				if (_resCtx->indices[i].hash == hash) {
					idx = i;
				}
			}
			if (idx == -1) {
				return false;
			}
			const ResourceIndex& res_idx = _resCtx->indices[idx];
			return (res_idx.type == type);
		}

		bool contains(RID rid, ResourceType type) {
			const ResourceIndex& res_idx = _resCtx->indices[rid];
			return res_idx.type == type;
		}

		RID find(StaticHash hash, ResourceType type) {
			int idx = -1;
			for (uint32_t i = 0; i < _resCtx->indices.size(); ++i) {
				if (_resCtx->indices[i].hash == hash) {
					idx = i;
				}
			}
			XASSERT(idx != -1, "No matching resource found for: %d", hash);
			const ResourceIndex& res_idx = _resCtx->indices[idx];
			XASSERT(res_idx.type == type, "Different resource types - expected %s but found %s", ResourceTypeNames[res_idx.type], ResourceTypeNames[type]);
			return res_idx.id;
		}

		RID find(const char* name, ResourceType type) {
			return find(SID(name),type);			
		}

		RID findBlendState(const char* name) {
			return find(name, ResourceType::BLENDSTATE);
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

		SquareBuffer* getSquareBuffer(RID rid) {
			const ResourceIndex& res_idx = _resCtx->indices[rid];
			XASSERT(res_idx.type == ResourceType::SQUAREBUFFER, "Different resource types - expected SQUREBUFFER but found %s", ResourceTypeNames[res_idx.type]);
			SquareBufferResource* res = static_cast<SquareBufferResource*>(_resCtx->resources[res_idx.id]);
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

		vm::Script* getScript(const char* name) {
			RID rid = find(name, ResourceType::SCRIPT);
			ScriptResource* res = static_cast<ScriptResource*>(_resCtx->resources[rid]);
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

		WorldEntityTemplates* getWorldEntityTemplates(StaticHash hash) {
			RID rid = find(hash, ResourceType::ENTITY_TEMPLATES);
			WorldEntityTemplatesResource* res = static_cast<WorldEntityTemplatesResource*>(_resCtx->resources[rid]);
			return res->get();
		}

		SpriteSheet* getSpriteSheet(const char* name) {
			RID rid = find(name, ResourceType::SPRITESHEET);
			SpriteSheetResource* res = static_cast<SpriteSheetResource*>(_resCtx->resources[rid]);
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