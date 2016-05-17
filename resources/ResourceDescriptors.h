#pragma once
#include <stdint.h>
#include <d3d11.h>
#include "..\Common.h"
#include <Vector.h>

namespace ds {

	struct ResourceDescriptor {
		uint16_t id;
		uint32_t nameIndex;
	};
	
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
		const char* geometryShader;
		const char* model;
		const char* file;
		RID samplerState;

		ShaderDescriptor() : vertexShader(0), pixelShader(0), geometryShader(0), model(0), file(0) {}
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

	struct SamplerStateDescriptor {
		uint16_t id;
		uint16_t addressU;
		uint16_t addressV;
		uint16_t addressW;
	};

	struct Shader {
		ID3D11VertexShader* vertexShader;
		ID3D11PixelShader* pixelShader;
		ID3D11GeometryShader* geometryShader;
		ID3DBlob* vertexShaderBuffer;
		ID3D11SamplerState* samplerState;
		ID3DBlob* geometryShaderBuffer;

		Shader() : vertexShader(0), pixelShader(0), vertexShaderBuffer(0), geometryShader(0), samplerState(0) {}

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

	struct QuadBufferDescriptor {
		uint16_t id;
		uint32_t size;
		RID indexBuffer;
		RID constantBuffer;
		RID vertexBuffer;
		RID shader;
		RID blendstate;
		RID colormap;
		RID inputlayout;
	};

	struct MeshBufferDescriptor {
		uint16_t id;
		uint32_t size;
		RID indexBuffer;
		RID constantBuffer;
		RID vertexBuffer;
		RID shader;
		RID blendstate;
		RID colormap;
		RID inputlayout;
	};

	struct MeshDescriptor {
		uint16_t id;
		const char* fileName;
		v3 position;
		v3 scale;
		v3 rotation;
	};

	struct WorldDescriptor {
		uint16_t id;
		uint32_t size;
		RID spriteBuffer;
	};

	struct IMGUIDescriptor {
		uint16_t id;
		RID font;
		RID spriteBuffer;
	};

	struct ParticleSystemsDescriptor {
		uint16_t id;
		RID spriteBuffer;
		uint32_t maxParticles;
	};

	struct GUIDialogDescriptor {
		uint16_t id;
		RID spriteBuffer;
		RID font;
		const char* file;
	};

}