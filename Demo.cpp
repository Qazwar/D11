#include "Demo.h"
#include "graphics.h"
#include "utils\Log.h"
#include "renderer\render_types.h"
#include "math\math.h"
#include <assert.h>


Demo::Demo() {
}


Demo::~Demo() {

}

bool Demo::initialize() {

	_shaderIndex = graphics::compileShader("TextureMap.fx");
	assert(_shaderIndex != -1);

	uint32_t data[] = { 0, 1, 3, 1, 2, 3, 4, 5, 7, 5, 6, 7 };
	_indexBuffer = graphics::createIndexBuffer(12, data);
	assert(_indexBuffer != -1);

	D3D11_INPUT_ELEMENT_DESC solidColorLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	_inputLayout = graphics::createInputLayout(_shaderIndex, solidColorLayout, 2);
	assert(_inputLayout != -1);

	_colorMap = graphics::loadTexture("content\\textures\\TextureArray.png");
	assert(_colorMap != -1);

	
	_vertexBuffer = graphics::createBuffer(sizeof(VertexPos) * 8);


	D3D11_BUFFER_DESC constDesc;
	ZeroMemory(&constDesc, sizeof(constDesc));
	constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constDesc.ByteWidth = sizeof(XMMATRIX);
	constDesc.Usage = D3D11_USAGE_DEFAULT;
	if (!graphics::createBuffer(constDesc, &_mvpCB)) {
		return false;
	}

	_positions[0] = v2(640.0f, 360.0f);
	_positions[1] = v2(400.0f, 200.0f);

	_alphaBlendState = graphics::createBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA);
	
	return true;
}

void Demo::shutdown() {
	if (_mvpCB) _mvpCB->Release();
	_mvpCB = 0;
}

void Demo::render() {
	unsigned int stride = sizeof(VertexPos);
	unsigned int offset = 0;
	ID3D11DeviceContext* ctx = graphics::getContext();

	graphics::setInputLayout(_inputLayout);
	graphics::setVertexBuffer(_vertexBuffer, &stride, &offset);
	graphics::setIndexBuffer(_indexBuffer);
	graphics::setBlendState(_alphaBlendState);
	

	graphics::setShader(_shaderIndex);
	graphics::setPixelShaderResourceView(_colorMap);

	XMMATRIX world = XMMatrixIdentity();
	XMMATRIX mvp = XMMatrixMultiply(world, graphics::getViewProjectionMaxtrix());
	mvp = XMMatrixTranspose(mvp);
	Texture t = math::buildTexture(70.0f, 0.0f, 260.0f, 60.0f);

	for (int i = 0; i < 2; i++) {
		float halfWidth = t.dim.x * 0.5f;
		float halfHeight = t.dim.y * 0.5f;
		v2 p = _positions[i];
		float left = (float)((1280 / 2) * -1) + p.x - halfWidth;
		float right = left + t.dim.x;
		float top = p.y - (float)(720 / 2) + halfHeight;
		float bottom = top - t.dim.y;
		_vertices[i * 4 + 0] = VertexPos(v3(left, top, 1.0f), t.getUV(0));
		_vertices[i * 4 + 1] = VertexPos(v3(right, top, 1.0f), t.getUV(1));
		_vertices[i * 4 + 2] = VertexPos(v3(right, bottom, 1.0f), t.getUV(2));
		_vertices[i * 4 + 3] = VertexPos(v3(left, bottom, 1.0f), t.getUV(3));
	}

	graphics::mapData(_vertexBuffer, _vertices, 8 * sizeof(VertexPos));
	ctx->UpdateSubresource(_mvpCB, 0, 0, &mvp, 0, 0);
	ctx->VSSetConstantBuffers(0, 1, &_mvpCB);

	ctx->DrawIndexed(12, 0, 0);//  (6, 0);
}