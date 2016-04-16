#pragma once
#include <stdint.h>
#include <d3d11.h>
#include "..\Common.h"

namespace ds {
	
	struct IndexBufferDescriptor {
		uint16_t id;
		uint32_t size;
		bool dynamic;
	};

	struct QuadIndexBufferDescriptor {

		uint16_t id;
		uint32_t size;

		QuadIndexBufferDescriptor() : id(0), size(0) {}
		QuadIndexBufferDescriptor(uint16_t _id, uint32_t _size) : id(_id), size(_size) {}

	};

	struct ConstantBufferDescriptor {

		uint16_t id;
		uint32_t size;

		ConstantBufferDescriptor() : id(0), size(0) {}
		ConstantBufferDescriptor(uint16_t _id, uint32_t _size) : id(_id), size(_size) {}
	};

	struct VertexBufferDescriptor {
		uint16_t id;
		uint32_t size;
		bool dynamic;
		uint32_t layout;
	};

	struct ShaderDescriptor {
		uint16_t id;
		const char* vertexShader;
		const char* pixelShader;
		const char* model;
		const char* file;
	};

	struct BlendStateDescriptor {
		uint16_t id;
		bool alphaEnabled;
		int srcBlend;
		int destBlend;
		int srcAlphaBlend;
		int destAlphaBlend;
	};

	struct TextureDescriptor {
		uint16_t id;
		const char* name;
	};

	struct Shader {
		ID3D11VertexShader* vertexShader;
		ID3D11PixelShader* pixelShader;
		ID3DBlob* vertexShaderBuffer;
		ID3D11SamplerState* samplerState;

		Shader() : vertexShader(0), pixelShader(0), vertexShaderBuffer(0), samplerState(0) {}

	};

	struct InputLayoutDescriptor {
		uint16_t id;
		uint16_t indices[16];
		uint16_t num;
		RID shader;
	};

	struct BitmapfontDescriptor {
		uint16_t id;
		const char* name;
	};

	struct SpriteBufferDescriptor {
		uint16_t id;
		uint32_t size;
		RID indexBuffer;
		RID constantBuffer;
		RID vertexBuffer;
		RID shader;
		RID blendstate;
		RID colormap;
		RID inputlayout;
		RID font;
	};

	struct WorldDescriptor {
		uint16_t id;
		uint32_t size;
		RID spriteBuffer;
	};

}