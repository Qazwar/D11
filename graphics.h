#pragma once
#include <stdint.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <DxErr.h>
#define _XM_NO_INTRINSICS_
#include <xnamath.h>

enum ResourceType {
	SHADER,
	TEXTURE,
	VERTEXBUFFER,
	INDEXBUFFER,
	UNKNOWN
};

typedef uint32_t RID;

const RID INVALID_RID = 0;

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

	bool initialize(HINSTANCE hInstance, HWND hwnd);

	ID3D11DeviceContext* getContext();

	int createConstantBuffer(uint32_t size);

	void updateConstantBuffer(int index, void* data);

	int compileShader(char* fileName);

	bool compileShader(char* filePath, char* entry, char* shaderModel, ID3DBlob** buffer);

	bool createVertexShader(ID3DBlob* buffer, ID3D11VertexShader** shader);

	bool createPixelShader(ID3DBlob* buffer, ID3D11PixelShader** shader);

	int createInputLayout(int shaderIndex, Attribute* attribute);

	int loadTexture(const char* name);

	bool createSamplerState(ID3D11SamplerState** sampler);

	const XMMATRIX& getViewProjectionMaxtrix();

	void setBlendState(int index);

	int createBlendState(D3D11_BLEND srcBlend, D3D11_BLEND destBlend);

	int createIndexBuffer(uint32_t num, uint32_t* data);

	int createQuadIndexBuffer(uint32_t numQuads);

	int createBuffer(uint32_t bufferSize, D3D11_SUBRESOURCE_DATA resourceData);

	int createBuffer(uint32_t bufferSize);

	bool createBuffer(D3D11_BUFFER_DESC bufferDesciption, D3D11_SUBRESOURCE_DATA resourceData, ID3D11Buffer** buffer);

	bool createBuffer(D3D11_BUFFER_DESC bufferDesciption, ID3D11Buffer** buffer);

	void beginRendering();

	void setIndexBuffer(int index);

	void setVertexBuffer(int index, uint32_t* stride, uint32_t* offset);

	void mapData(int bufferIndex, void* data, uint32_t size);

	void setShader(int shaderIndex);

	void setInputLayout(int layoutIndex);

	void setPixelShaderResourceView(int index, uint32_t slot = 0);

	void setVertexShaderConstantBuffer(int bufferIndex);

	void drawIndexed(int num);

	void endRendering();

	void shutdown();
}

