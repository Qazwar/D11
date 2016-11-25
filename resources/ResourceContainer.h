#pragma once
#include <d3d11.h>
#include <vector>
#include <map>
#include "ResourceDescriptors.h"
#include "core\Common.h"
#include "..\renderer\Bitmapfont.h"
#include "..\renderer\sprites.h"
#include "..\particles\ParticleManager.h"
#include "..\dialogs\GUIDialog.h"
#include "..\renderer\QuadBuffer.h"
#include "..\renderer\MeshBuffer.h"
#include "..\scene\Scene.h"
#include "..\renderer\SkyBox.h"
#include "..\renderer\RenderTarget.h"
#include "..\renderer\SpriteSheet.h"
#include "..\renderer\SquareBuffer.h"
#include <core\world\WorldEntityTemplates.h>
#include <core\script\vm.h>
#include "Resource.h"

namespace ds {

	enum Attribute {
		Position,
		Color0,
		Texcoord0,
		Texcoord1,
		Texcoord2,
		Texcoord3,
		End
	};

	const uint32_t MAX_RESOURCES = 1024;

	enum ResourceType {
		SHADER,
		TEXTURE,
		VERTEXBUFFER,
		INDEXBUFFER,
		CONSTANTBUFFER,
		BLENDSTATE,
		INPUTLAYOUT,
		BITMAPFONT,
		SPRITEBUFFER,
		WORLD,
		SAMPLERSTATE,
		GUIDIALOG,
		QUADBUFFER,
		MESHBUFFER,
		MESH,
		SCENE,
		CAMERA,
		RENDERTARGET,
		SKYBOX,
		MATERIAL,
		PARTICLEMANAGER,
		SPRITESHEET,
		SOUND,
		SCRIPT,
		ENTITY_TEMPLATES,
		SQUAREBUFFER,
		UNKNOWN
	};

	namespace res {

		typedef void(*ParseFunc)(JSONReader&, int);

		// ------------------------------------------------------
		// resource index
		// ------------------------------------------------------
		struct ResourceIndex {
			RID id;
			ResourceType type;
			uint32_t nameIndex;
			StaticHash hash;
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
			ParticleManager* particles;

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
			"SPRITESHEET",
			"SOUND",
			"SCRIPT",
			"ENTITY_TEMPLATES",
			"SQUAREBUFFER",
			"UNKNOWN"
		};

		void initialize(ID3D11Device* device);

		void shutdown();

		int findBlendStateMapping(const char* text);

		RID findBlendState(const char* name);

		RID createShader(const char* name, const ShaderDescriptor& descriptor);

		RID createEmptyShader(const char* name);

		RID createInputLayout(const char* name, const InputLayoutDescriptor& descriptor);

		RID createVertexBuffer(const char* name, const VertexBufferDescriptor& descriptor);

		RID createBlendState(const char* name, const BlendStateDescriptor& descriptor);

		RID createMaterial(const char* name, const MaterialDescriptor& descriptor);

		RID createSamplerState(const char* name, const SamplerStateDescriptor& descriptor);

		RID createConstantBuffer(const char* name, const ConstantBufferDescriptor& descriptor);

		void reloadDialog(const char* name);

		void parseJSONFile();

		RID find(const char* name, ResourceType type);

		BaseResource* getResource(RID rid, ResourceType type);

		bool contains(StaticHash hash, ResourceType type);

		bool contains(RID rid, ResourceType type);

		const char* getName(RID rid);

		ID3D11Buffer* getIndexBuffer(RID rid);

		ID3D11BlendState* getBlendState(RID rid);

		ID3D11Buffer* getConstantBuffer(RID rid);

		ID3D11Buffer* getConstantBuffer(const char* name);

		ID3D11Buffer* getVertexBuffer(RID rid);

		ID3D11InputLayout* getInputLayout(RID rid);

		ID3D11InputLayout* getInputLayout(const char* name);

		ID3D11ShaderResourceView* getShaderResourceView(RID rid);

		ID3D11SamplerState* getSamplerState(RID rid);

		ParticleManager* getParticleManager();

		Bitmapfont* getFont(RID rid);

		Shader* getShader(RID rid);

		GUIDialog* getGUIDialog(RID rid);

		GUIDialog* getGUIDialog(const char* name);

		SpriteSheet* getSpriteSheet(const char* name);

		WorldEntityTemplates* getWorldEntityTemplates(StaticHash hash);

		SpriteBuffer* getSpriteBuffer(const char* name);

		SpriteBuffer* getSpriteBuffer(RID rid);

		QuadBuffer* getQuadBuffer(RID rid);

		MeshBuffer* getMeshBuffer(RID rid);

		MeshBuffer* getMeshBuffer(const char* name);

		Material* getMaterial(const char* name);

		Material* getMaterial(RID rid);

		Scene* getScene(const char* name);

		RenderTarget* getRenderTarget(RID rid);

		Mesh* getMesh(RID rid);

		Mesh* getMesh(const char* name);

		SquareBuffer* getSquareBuffer(RID rid);

		SkyBox* getSkyBox(const char* name);

		vm::Script* getScript(const char* name);

		int findInputElement(const char* name);

		void debug();

		void save(const ReportWriter& rw);

	}
	
}

