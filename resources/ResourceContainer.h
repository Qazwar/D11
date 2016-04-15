#pragma once
#include <d3d11.h>
#include <vector>
#include "ResourceDescriptors.h"
#include "..\Common.h"
#include "..\renderer\Bitmapfont.h"
#include "..\renderer\sprites.h"

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
		UNKNOWN
	};


	class ResourceContainer {

	struct ResourceIndex {
		RID id;
		uint32_t index;
		ResourceType type;
	};

	public:
		ResourceContainer(ID3D11Device* d3dDevice);
		~ResourceContainer();
		void parseJSONFile();
		RID createQuadIndexBuffer(const QuadIndexBufferDescriptor& descriptor);
		RID createConstantBuffer(const ConstantBufferDescriptor& descriptor);
		RID loadTexture(const TextureDescriptor& descriptor);
		RID createBlendState(const BlendStateDescriptor& descriptor);
		RID createIndexBuffer(const IndexBufferDescriptor& descriptor);
		RID createVertexBuffer(const VertexBufferDescriptor& descriptor);		
		RID createShader(const ShaderDescriptor& descriptor);
		RID createInputLayout(const InputLayoutDescriptor& descriptor);
		RID loadFont(const BitmapfontDescriptor& descriptor);
		RID createSpriteBuffer(const SpriteBufferDescriptor& descriptor);
		ID3D11Buffer* getIndexBuffer(RID rid);
		ID3D11BlendState* getBlendState(RID rid);
		ID3D11Buffer* getConstantBuffer(RID rid);
		ID3D11Buffer* getVertexBuffer(RID rid);
		ID3D11InputLayout* getInputLayout(RID rid);
		ID3D11ShaderResourceView* getShaderResourceView(RID rid);
		Bitmapfont* getFont(RID rid);
		Shader* getShader(RID rid);
		SpriteBuffer* getSpriteBuffer(RID rid);
		int findInputElement(const char* name);
		int findBlendState(const char* text);
	private:
		bool compileShader(const char* filePath, const  char* entry, const  char* shaderModel, ID3DBlob** buffer);
		bool createVertexShader(ID3DBlob* buffer, ID3D11VertexShader** shader);
		bool createPixelShader(ID3DBlob* buffer, ID3D11PixelShader** shader);

		ID3D11Device* _device;
		std::vector<ID3D11Buffer*> _indexBuffers;
		std::vector<ID3D11Buffer*> _constantBuffers;
		std::vector<ID3D11ShaderResourceView*> _shaderResourceViews;
		std::vector<ID3D11BlendState*> _blendStates;
		std::vector<ID3D11Buffer*> _vertexBuffers;
		std::vector<Shader*> _shaders;
		std::vector<ID3D11InputLayout*> _layouts;
		std::vector<Bitmapfont*> _fonts;
		std::vector<SpriteBuffer*> _spriteBuffers;
		uint32_t _resourceIndex;
		ResourceIndex _resourceTable[MAX_RESOURCES];
	};

}

