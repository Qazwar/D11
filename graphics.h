#pragma once
#include <stdint.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <DxErr.h>

namespace graphics {

	bool initialize(HINSTANCE hInstance, HWND hwnd);

	ID3D11DeviceContext* getContext();

	bool compileShader(char* filePath, char* entry, char* shaderModel, ID3DBlob** buffer);

	bool createVertexShader(ID3DBlob* buffer, ID3D11VertexShader** shader);

	bool createPixelShader(ID3DBlob* buffer, ID3D11PixelShader** shader);

	bool createInputLayout(ID3DBlob* buffer, D3D11_INPUT_ELEMENT_DESC* descriptors, uint32_t num, ID3D11InputLayout** layout);

	bool loadTexture(const char* name, ID3D11ShaderResourceView** srv);

	bool createSamplerState(ID3D11SamplerState** sampler);

	bool createBlendState(ID3D11BlendState** state);

	void setBlendState(int index);

	int createBlendState(D3D11_BLEND srcBlend, D3D11_BLEND destBlend);

	int createIndexBuffer(uint32_t num, uint32_t* data);

	bool createBuffer(D3D11_BUFFER_DESC vertexDesc, D3D11_SUBRESOURCE_DATA resourceData, ID3D11Buffer** buffer);

	bool createBuffer(D3D11_BUFFER_DESC bufferDesciption, ID3D11Buffer** buffer);

	void beginRendering();

	void setIndexBuffer(int index);

	void endRendering();

	void shutdown();
}

