#pragma once
#include <stdint.h>
#include <d3d11.h>
#include "..\Common.h"
#include "..\utils\Color.h"
#include <Vector.h>

namespace ds {

	struct ResourceDescriptor {
		uint32_t nameIndex;
		IdString hash;
	};
	
	struct IndexBufferDescriptor {
		uint32_t size;
		bool dynamic;
	};

	struct QuadIndexBufferDescriptor {

		uint32_t size;

		QuadIndexBufferDescriptor() : size(0) {}
		QuadIndexBufferDescriptor(uint32_t _size) : size(_size) {}

	};

	struct ConstantBufferDescriptor {

		uint32_t size;

		ConstantBufferDescriptor() : size(0) {}
		ConstantBufferDescriptor(uint32_t _size) : size(_size) {}
	};

	struct VertexBufferDescriptor {
		uint32_t size;
		bool dynamic;
		uint32_t layout;
	};

	struct ShaderDescriptor {
		const char* vertexShader;
		const char* pixelShader;
		const char* geometryShader;
		const char* model;
		const char* file;
		RID samplerState;

		ShaderDescriptor() : vertexShader(0), pixelShader(0), geometryShader(0), model(0), file(0) {}
	};

	struct BlendStateDescriptor {
		bool alphaEnabled;
		int srcBlend;
		int destBlend;
		int srcAlphaBlend;
		int destAlphaBlend;
	};

	struct TextureDescriptor {
		const char* name;
	};

	struct SamplerStateDescriptor {
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
		uint16_t indices[16];
		uint16_t num;
		RID shader;
		const BYTE* byteCode;
		uint32_t byteCodeSize;
	};

	struct BitmapfontDescriptor {
		const char* name;
	};

	struct SpriteBufferDescriptor {
		uint32_t size;
		RID indexBuffer;
		RID constantBuffer;
		RID vertexBuffer;
		RID material;
		RID font;
	};

	struct QuadBufferDescriptor {
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
		uint32_t size;
		RID indexBuffer;
		RID constantBuffer;
		RID vertexBuffer;
		//RID shader;
		//RID blendstate;
		//RID colormap;
		//RID inputlayout;
		RID material;
	};

	struct MeshDescriptor {
		const char* fileName;
		v3 position;
		v3 scale;
		v3 rotation;
	};

	struct IMGUIDescriptor {
		RID font;
		RID spriteBuffer;
	};

	struct ParticleSystemsDescriptor {
		RID spriteBuffer;
		uint32_t maxParticles;
	};

	struct GUIDialogDescriptor {
		RID font;
		const char* file;
	};

	struct CameraDescriptor {
		const char* type;
		v3 position;
		v3 target;
	};

	struct SceneDescriptor {
		const char* meshBuffer;
		uint16_t size;
		const char* camera;
		bool depthEnabled;
	};

	struct RenderTargetDescriptor {
	};

	struct SkyBoxDescriptor {
		uint32_t size;
		RID indexBuffer;
		RID constantBuffer;
		RID vertexBuffer;
		RID shader;
		RID blendstate;
		RID colormap;
		RID inputlayout;
		float scale;
	};

	struct MaterialDescriptor {
		uint32_t size;
		RID shader;
		RID blendstate;
		RID texture;
		Color diffuse;
		Color ambient;
	};

}