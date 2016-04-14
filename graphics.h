#pragma once
#include <stdint.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <DxErr.h>
#include "base\Settings.h"
#include "math\math_types.h"
#include "resources\ResourceDescriptors.h"

const uint32_t MAX_RESOURCES = 1024;

enum ResourceType {
	SHADER,
	TEXTURE,
	VERTEXBUFFER,
	INDEXBUFFER,
	CONSTANTBUFFER,
	BLENDSTATE,
	UNKNOWN
};

typedef uint32_t RID;

const RID INVALID_RID = UINT32_MAX;

struct Access{

	enum ENUM {
		Read,
		Write,
		ReadWrite,
		Count
	};
};

enum Attribute {
	Position,
	Color0,
	Texcoord0,
	Texcoord1,
	Texcoord2,
	Texcoord3,
	End
};

namespace graphics {

	struct Shader {
		ID3D11VertexShader* vertexShader;
		ID3D11PixelShader* pixelShader;
		ID3DBlob* vertexShaderBuffer;
		ID3D11SamplerState* samplerState;

		Shader() : vertexShader(0), pixelShader(0), vertexShaderBuffer(0) , samplerState(0) {}

	};

	bool initialize(HINSTANCE hInstance, HWND hwnd, const ds::Settings& settings);

	ID3D11DeviceContext* getContext();

	RID createConstantBuffer(const ConstantBufferDescriptor& descriptor);

	void updateConstantBuffer(RID rid, void* data);

	RID createShader(const ShaderDescriptor& descriptor);

	//int compileShader(char* fileName);

	bool compileShader(const char* filePath, const char* entry, const char* shaderModel, ID3DBlob** buffer);

	bool createVertexShader(ID3DBlob* buffer, ID3D11VertexShader** shader);

	bool createPixelShader(ID3DBlob* buffer, ID3D11PixelShader** shader);

	int createInputLayout(RID shaderRID, Attribute* attribute);

	int loadTexture(const char* name);

	bool createSamplerState(ID3D11SamplerState** sampler);

	const ds::mat4& getViewProjectionMaxtrix();

	void setBlendState(int index);

	int createBlendState(D3D11_BLEND srcBlend, D3D11_BLEND destBlend);

	RID createIndexBuffer(const IndexBufferDescriptor& descriptor);

	RID createQuadIndexBuffer(const QuadIndexBufferDescriptor& descriptor);

	RID createVertexBuffer(const VertexBufferDescriptor& descriptor);

	void beginRendering(const Color& color);

	void setIndexBuffer(RID rid);

	void setVertexBuffer(RID rid, uint32_t* stride, uint32_t* offset);

	void mapData(RID rid, void* data, uint32_t size);

	void setShader(RID rid);

	void setInputLayout(int layoutIndex);

	void setPixelShaderResourceView(int index, uint32_t slot = 0);

	void setVertexShaderConstantBuffer(RID rid);

	void drawIndexed(int num);

	void endRendering();

	void shutdown();
}

