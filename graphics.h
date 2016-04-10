#pragma once
#include <stdint.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <DxErr.h>
#include <xnamath.h>

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

	int compileShader(char* fileName);

	bool compileShader(char* filePath, char* entry, char* shaderModel, ID3DBlob** buffer);

	bool createVertexShader(ID3DBlob* buffer, ID3D11VertexShader** shader);

	bool createPixelShader(ID3DBlob* buffer, ID3D11PixelShader** shader);

	bool createInputLayout(ID3DBlob* buffer, D3D11_INPUT_ELEMENT_DESC* descriptors, uint32_t num, ID3D11InputLayout** layout);

	int createInputLayout(int shaderIndex, D3D11_INPUT_ELEMENT_DESC* descriptors, uint32_t num);

	int loadTexture(const char* name);

	bool createSamplerState(ID3D11SamplerState** sampler);

	bool createBlendState(ID3D11BlendState** state);

	const XMMATRIX& getViewProjectionMaxtrix();

	void setBlendState(int index);

	int createBlendState(D3D11_BLEND srcBlend, D3D11_BLEND destBlend);

	int createIndexBuffer(uint32_t num, uint32_t* data);

	bool createBuffer(D3D11_BUFFER_DESC vertexDesc, D3D11_SUBRESOURCE_DATA resourceData, ID3D11Buffer** buffer);

	bool createBuffer(D3D11_BUFFER_DESC bufferDesciption, ID3D11Buffer** buffer);

	void beginRendering();

	void setIndexBuffer(int index);

	void setShader(int shaderIndex);

	void setInputLayout(int layoutIndex);

	void setPixelShaderResourceView(int index, uint32_t slot = 0);

	void endRendering();

	void shutdown();
}

