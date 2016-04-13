#include "sprites.h"
#include "..\graphics.h"
#include <assert.h>

SpriteBuffer::SpriteBuffer(int maxSprites) : _maxSprites(maxSprites) , _index(0) , _started(false) {
	// create indexbuffer
	_indexBuffer = graphics::createQuadIndexBuffer(maxSprites * 6);
	assert(_indexBuffer != -1);
	// create shader
	_shaderIndex = graphics::compileShader("TextureMap.fx");
	// create input layout
	Attribute desc[] = { Attribute::Position, Attribute::Texcoord0, Attribute::Color0 , Attribute::End };
	_layoutIndex = graphics::createInputLayout(_shaderIndex, desc);
	assert(_layoutIndex != -1);
	// load texture
	_colorMap = graphics::loadTexture("content\\textures\\TextureArray.png");
	// create vertexbuffer
	_vertexBuffer = graphics::createBuffer(sizeof(SpriteVertex) * 4 * maxSprites);
	// create constant buffer
	_mvpCB = graphics::createConstantBuffer(sizeof(XMMATRIX));
	// create blend state
	_alphaBlendState = graphics::createBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA);
	// create data
	_sprites = new Sprite[maxSprites];
	_vertices = new SpriteVertex[4 * maxSprites];
}

SpriteBuffer::~SpriteBuffer() {
}

void SpriteBuffer::draw(const v2& position, const Texture& texture, const Color& color) {
	if (_started) {
		if (_index >= _maxSprites) {
			flush();
		}
		Sprite& sprite = _sprites[_index++];
		sprite.position = position;
		sprite.texture = texture;
		sprite.color = color;
	}
}

void SpriteBuffer::begin() {
	_index = 0;
	_started = true;
}

void SpriteBuffer::end() {
	flush();
	_started = false;
}

void SpriteBuffer::flush() {
	unsigned int stride = sizeof(SpriteVertex);
	unsigned int offset = 0;

	graphics::setInputLayout(_layoutIndex);
	graphics::setVertexBuffer(_vertexBuffer, &stride, &offset);
	graphics::setIndexBuffer(_indexBuffer);
	graphics::setBlendState(_alphaBlendState);
	graphics::setShader(_shaderIndex);
	graphics::setPixelShaderResourceView(_colorMap);

	XMMATRIX world = XMMatrixIdentity();
	XMMATRIX mvp = XMMatrixMultiply(world, graphics::getViewProjectionMaxtrix());
	mvp = XMMatrixTranspose(mvp);

	for (int i = 0; i < _index; i++) {
		const Sprite& sprite = _sprites[i];
		float halfWidth = sprite.texture.dim.x * 0.5f;
		float halfHeight = sprite.texture.dim.y * 0.5f;
		v2 p = sprite.position;
		float left = (float)((1280 / 2) * -1) + p.x - halfWidth;
		float right = left + sprite.texture.dim.x;
		float top = p.y - (float)(720 / 2) + halfHeight;
		float bottom = top - sprite.texture.dim.y;
		_vertices[i * 4 + 0] = SpriteVertex(v2(left, top)    , sprite.texture.getUV(0), sprite.color);
		_vertices[i * 4 + 1] = SpriteVertex(v2(right, top)   , sprite.texture.getUV(1), sprite.color);
		_vertices[i * 4 + 2] = SpriteVertex(v2(right, bottom), sprite.texture.getUV(2), sprite.color);
		_vertices[i * 4 + 3] = SpriteVertex(v2(left, bottom) , sprite.texture.getUV(3), sprite.color);
	}

	graphics::mapData(_vertexBuffer, _vertices, _index * 4 * sizeof(SpriteVertex));
	graphics::updateConstantBuffer(_mvpCB, &mvp);
	graphics::setVertexShaderConstantBuffer(_mvpCB);

	graphics::drawIndexed(_index * 6);
	_index = 0;
}