#include "Demo.h"
#include "graphics.h"
#include <Vector.h>
#include "renderer\render_types.h"
#include "math\math.h"

struct VertexPos {
	v3 pos;
	v2 tex0;
};

Demo::Demo() {
}


Demo::~Demo() {

}

bool Demo::initialize() {

	ID3DBlob* vsBuffer = 0;
	bool compileResult = graphics::compileShader("TextureMap.fx", "VS_Main", "vs_4_0", &vsBuffer);
	if (!compileResult)	{
		DXTRACE_MSG("Error compiling the vertex shader!");
		return false;
	}
	HRESULT d3dResult;

	if (!graphics::createVertexShader(vsBuffer, &_solidColorVS)) {
		DXTRACE_MSG("Error creating the vertex shader!");
		return false;
	}

	uint32_t data[] = { 0, 1, 3, 1, 2, 3 };
	_indexBuffer = graphics::createIndexBuffer(6, data);
	if (_indexBuffer == -1) {
		return false;
	}

	D3D11_INPUT_ELEMENT_DESC solidColorLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	if (!graphics::createInputLayout(vsBuffer, solidColorLayout, 2, &_inputLayout)) {
		DXTRACE_MSG("Error creating the input layout!");
		return false;
	}

	vsBuffer->Release();

	ID3DBlob* psBuffer = 0;

	compileResult = graphics::compileShader("TextureMap.fx", "PS_Main", "ps_4_0", &psBuffer);

	if (!compileResult)	{
		DXTRACE_MSG("Error compiling pixel shader!");
		return false;
	}

	if (!graphics::createPixelShader(psBuffer, &_solidColorPS)) {
		DXTRACE_MSG("Error creating pixel shader!");
		return false;
	}

	psBuffer->Release();

	if (!graphics::loadTexture("content\\textures\\TextureArray.png", &_colorMap)) {
		return false;
	}

	if (!graphics::createSamplerState(&_colorMapSampler)) {
		return false;
	}
	/*
	ID3D11Resource* colorTex;
	_colorMap->GetResource(&colorTex);

	D3D11_TEXTURE2D_DESC colorTexDesc;
	((ID3D11Texture2D*)colorTex)->GetDesc(&colorTexDesc);
	colorTex->Release();

	float halfWidth = (float)colorTexDesc.Width / 2.0f;
	float halfHeight = (float)colorTexDesc.Height / 2.0f;
	*/

	Texture t = math::buildTexture(70.0f, 0.0f, 260.0f, 60.0f);
	float halfWidth = t.dim.x * 0.5f;
	float halfHeight = t.dim.y * 0.5f;
	VertexPos vertices[] =
	{
		{ v3(-halfWidth,  halfHeight, 1.0f), t.getUV(0) },
		{ v3( halfWidth,  halfHeight, 1.0f), t.getUV(1) },
		{ v3( halfWidth, -halfHeight, 1.0f), t.getUV(2) },
		{ v3(-halfWidth, -halfHeight, 1.0f), t.getUV(3) },
	};

	D3D11_BUFFER_DESC vertexDesc;
	ZeroMemory(&vertexDesc, sizeof(vertexDesc));
	vertexDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexDesc.ByteWidth = sizeof(VertexPos) * 4;

	D3D11_SUBRESOURCE_DATA resourceData;
	ZeroMemory(&resourceData, sizeof(resourceData));
	resourceData.pSysMem = vertices;

	if (!graphics::createBuffer(vertexDesc, resourceData, &_vertexBuffer)) {
		return false;
	}


	D3D11_BUFFER_DESC constDesc;
	ZeroMemory(&constDesc, sizeof(constDesc));
	constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constDesc.ByteWidth = sizeof(XMMATRIX);
	constDesc.Usage = D3D11_USAGE_DEFAULT;
	if (!graphics::createBuffer(constDesc, &_mvpCB)) {
		return false;
	}


	_positions[0] = XMFLOAT2(400.0f, 300.0f);
	_positions[1] = XMFLOAT2(400.0f, 100.0f);

	XMMATRIX view = XMMatrixIdentity();
	XMMATRIX projection = XMMatrixOrthographicOffCenterLH(0.0f, 800.0f, 0.0f, 600.0f, 0.1f, 100.0f);
	_vpMatrix = XMMatrixMultiply(view, projection);


	_alphaBlendState = graphics::createBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA);
	
	return true;
}

void Demo::shutdown() {
	if (_colorMapSampler) _colorMapSampler->Release();
	if (_colorMap) _colorMap->Release();
	if (_solidColorVS) _solidColorVS->Release();
	if (_solidColorPS) _solidColorPS->Release();
	if (_inputLayout) _inputLayout->Release();
	if (_vertexBuffer) _vertexBuffer->Release();
	if (_mvpCB) _mvpCB->Release();

	_colorMapSampler = 0;
	_colorMap = 0;
	_solidColorVS = 0;
	_solidColorPS = 0;
	_inputLayout = 0;
	_vertexBuffer = 0;
	_mvpCB = 0;
}

void Demo::render() {
	unsigned int stride = sizeof(VertexPos);
	unsigned int offset = 0;
	ID3D11DeviceContext* ctx = graphics::getContext();

	ctx->IASetInputLayout(_inputLayout);
	ctx->IASetVertexBuffers(0, 1, &_vertexBuffer, &stride, &offset);
	graphics::setIndexBuffer(_indexBuffer);
	graphics::setBlendState(_alphaBlendState);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ctx->VSSetShader(_solidColorVS, 0, 0);
	ctx->PSSetShader(_solidColorPS, 0, 0);
	ctx->PSSetShaderResources(0, 1, &_colorMap);
	ctx->PSSetSamplers(0, 1, &_colorMapSampler);

	for (int i = 0; i < 1; i++) {

		XMMATRIX translation = XMMatrixTranslation(_positions[i].x, _positions[i].y, 0.0f);
		//XMMATRIX rotationZ = XMMatrixRotationZ(rotation_);
		//XMMATRIX scale = XMMatrixScaling(scale_.x, scale_.y, 1.0f);
		//return translation * rotationZ * scale;
		XMMATRIX world = translation;
		XMMATRIX mvp = XMMatrixMultiply(world, _vpMatrix);
		mvp = XMMatrixTranspose(mvp);

		ctx->UpdateSubresource(_mvpCB, 0, 0, &mvp, 0, 0);
		ctx->VSSetConstantBuffers(0, 1, &_mvpCB);

		ctx->DrawIndexed(6, 0, 0);//  (6, 0);
	}
}