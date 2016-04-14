#include "sprites.h"
#include "..\graphics.h"
#include <assert.h>
#include "renderQueue.h"
#include "..\math\math_types.h"
#include "..\math\matrix.h"
#include "..\resources\ResourceDescriptors.h"
#include "..\utils\Log.h"

const v2 ARRAY[] = { v2(-0.5f, 0.5f), v2(0.5f, 0.5f), v2(0.5f, -0.5f), v2(-0.5f, -0.5f) };

v2 srt(const v2& v, const v2& u, float scaleX, float scaleY, float rotation) {
	float sx = u.x * scaleX;
	float sy = u.y * scaleY;
	// rotation clock wise
	//float xt = cosf(rotation) * sx + sinf(rotation) * sy;
	//float yt = -sinf(rotation) * sx + cosf(rotation) * sy;
	// rotation counter clock wise
	float xt = cos(rotation) * sx - sin(rotation) * sy;
	float yt = sin(rotation) * sx + cos(rotation) * sy;
	xt += v.x;
	yt += v.y;
	return Vector2f(xt, yt);
}

SpriteBuffer::SpriteBuffer(int maxSprites) : _maxSprites(maxSprites) , _index(0) , _started(false) {
	_indexBuffer = 0;
	// create shader
	_shaderIndex = 3;
	// create input layout
	Attribute desc[] = { Attribute::Position, Attribute::Texcoord0, Attribute::Color0 , Attribute::End };
	_layoutIndex = graphics::createInputLayout(_shaderIndex, desc);
	assert(_layoutIndex != -1);
	// load texture
	_colorMap = graphics::loadTexture("content\\textures\\TextureArray.png");

	_vertexBuffer = 2;
	_mvpCB = 1;
	// create blend state
	_alphaBlendState = 4;// graphics::createBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA);
	// create data
	_sprites = new Sprite[maxSprites];
	_vertices = new SpriteVertex[4 * maxSprites];
}

SpriteBuffer::~SpriteBuffer() {
}

void SpriteBuffer::draw(const Sprite& sprite) {
	draw(sprite.position, sprite.texture, sprite.rotation, sprite.scale, sprite.color);
}

void SpriteBuffer::draw(const v2& position, const Texture& texture, float rotation, const v2& scale, const Color& color) {
	if (_started) {
		if (_index >= _maxSprites) {
			flush();
		}
		Sprite& sprite = _sprites[_index++];
		sprite.position = position;
		sprite.texture = texture;
		sprite.color = color;
		sprite.scale = scale;
		sprite.rotation = rotation;
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

	//XMMATRIX world = XMMatrixIdentity();
	ds::mat4 mvp = graphics::getViewProjectionMaxtrix();
	mvp = ds::matrix::mat4Transpose(mvp);
	

	for (int i = 0; i < _index; i++) {
		const Sprite& sprite = _sprites[i];
		for (int j = 0; j < 4; ++j) {
			v2 start = ARRAY[j];
			start.x *= sprite.texture.dim.x;
			start.y *= sprite.texture.dim.y;
			v2 sp = sprite.position;
			sp -= v2(640, 360);
			v2 p = srt(sp, start, sprite.scale.x, sprite.scale.y, sprite.rotation);
			_vertices[i * 4 + j] = SpriteVertex(p, sprite.texture.getUV(j), sprite.color);
		}
	}

	graphics::mapData(_vertexBuffer, _vertices, _index * 4 * sizeof(SpriteVertex));
	
	graphics::updateConstantBuffer(_mvpCB, &mvp);
	graphics::setVertexShaderConstantBuffer(_mvpCB);

	graphics::drawIndexed(_index * 6);

	_index = 0;
}