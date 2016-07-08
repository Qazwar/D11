#pragma once
#include <d3d11.h>
#include <vector>
#include "ResourceDescriptors.h"
#include "..\Common.h"
#include "..\renderer\Bitmapfont.h"
#include "..\renderer\sprites.h"
#include "..\particles\ParticleManager.h"
#include "..\dialogs\GUIDialog.h"
#include "..\renderer\QuadBuffer.h"
#include "..\renderer\MeshBuffer.h"
#include "..\scene\Scene.h"
#include "..\renderer\SkyBox.h"
#include "..\renderer\RenderTarget.h"
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
		UNKNOWN
	};

	namespace res {

		void initialize(ID3D11Device* device);

		void shutdown();

		void parseJSONFile();

		RID find(const char* name, ResourceType type);

		BaseResource* getResource(RID rid, ResourceType type);

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

		SpriteBuffer* getSpriteBuffer(const char* name);

		SpriteBuffer* getSpriteBuffer(RID rid);

		QuadBuffer* getQuadBuffer(RID rid);

		MeshBuffer* getMeshBuffer(RID rid);

		MeshBuffer* getMeshBuffer(const char* name);

		Material* getMaterial(const char* name);

		Material* getMaterial(RID rid);

		Scene* getScene(const char* name);

		Mesh* getMesh(RID rid);

		Mesh* getMesh(const char* name);

		SkyBox* getSkyBox(const char* name);

		int findInputElement(const char* name);

		int findBlendState(const char* text);

		void debug();

		void save(const ReportWriter& rw);

	}
	
}

