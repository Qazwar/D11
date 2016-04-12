#include "sprites.h"
#include "..\graphics.h"

const v2 ARRAY[] = { v2(-0.5f, 0.5f), v2(0.5f, 0.5f), v2(0.5f, -0.5f), v2(-0.5f, -0.5f) };

v2 srt(const v2& v, const v2& u, float scaleX, float scaleY, float rotation) {
	float sx = u.x * scaleX;
	float sy = u.y * scaleY;

	// rotation clock wise
	//float xt = cosf(rotation) * sx + sinf(rotation) * sy;
	//float yt = -sinf(rotation) * sx + cosf(rotation) * sy;

	// rotation counter clock wise
	//float xt = cosf(rotation) * sx - sinf(rotation) * sy;
	//float yt = sinf(rotation) * sx + cosf(rotation) * sy;

	float xt = cos(rotation) * sx -sin(rotation) * sy;
	float yt = sin(rotation) * sx + cos(rotation) * sy;

	xt += v.x;
	yt += v.y;

	return Vector2f(xt, yt);
}

SpriteBuffer::SpriteBuffer(int maxSprites) : _maxSprites(maxSprites) , _index(0) , _started(false) {
	// create indexbuffer
	int size = maxSprites * 6;
	uint32_t* data = new uint32_t[size];
	int base = 0;
	int cnt = 0;
	for (int i = 0; i < maxSprites; ++i) {
		data[base] = cnt;
		data[base + 1] = cnt + 1;
		data[base + 2] = cnt + 3;
		data[base + 3] = cnt + 1;
		data[base + 4] = cnt + 2;
		data[base + 5] = cnt + 3;
		base += 6;
		cnt += 4;
	}
	_indexBuffer = graphics::createIndexBuffer(size, data);
	if (_indexBuffer == -1) {
	}
	delete[] data;

	_shaderIndex = graphics::compileShader("TextureMap.fx");

	// create layout
	D3D11_INPUT_ELEMENT_DESC spriteLayout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	_layoutIndex = graphics::createInputLayout(_shaderIndex, spriteLayout, 2);

	_colorMap = graphics::loadTexture("content\\textures\\TextureArray.png");

	_vertexBuffer = graphics::createBuffer(sizeof(SpriteVertex) * 4 * maxSprites);

	_mvpCB = graphics::createConstantBuffer(sizeof(XMMATRIX));
	
	_alphaBlendState = graphics::createBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA);

	_sprites = new Sprite[maxSprites];
	_vertices = new SpriteVertex[4 * maxSprites];
}

SpriteBuffer::~SpriteBuffer() {
}

void SpriteBuffer::draw(const v2& position, const Texture& texture, float rotation, const v2& scale) {
	if (_started) {
		if (_index >= _maxSprites) {
			flush();
		}
		Sprite& sprite = _sprites[_index++];
		sprite.position = position;
		sprite.texture = texture;
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
	ID3D11DeviceContext* ctx = graphics::getContext();

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
		float halfWidth = sprite.texture.dim.x;// *0.5f;
		float halfHeight = sprite.texture.dim.y;// *0.5f;
		for (int j = 0; j < 4; ++j) {
			v2 start = ARRAY[j];
			start.x *= halfWidth;
			start.y *= halfHeight;
			v2 sp = sprite.position;
			sp -= v2(640, 360);
			v2 p = srt(sp, start, sprite.scale.x, sprite.scale.y, sprite.rotation);
			_vertices[i * 4 + j] = SpriteVertex(p, sprite.texture.getUV(j));
		}
		/*
		v2 p = sprite.position;
		float left = (float)((1280 / 2) * -1) + p.x - halfWidth;
		float right = left + sprite.texture.dim.x;
		float top = p.y - (float)(720 / 2) + halfHeight;
		float bottom = top - sprite.texture.dim.y;
		_vertices[i * 4 + 0] = SpriteVertex(v2(left, top), sprite.texture.getUV(0));
		_vertices[i * 4 + 1] = SpriteVertex(v2(right, top), sprite.texture.getUV(1));
		_vertices[i * 4 + 2] = SpriteVertex(v2(right, bottom), sprite.texture.getUV(2));
		_vertices[i * 4 + 3] = SpriteVertex(v2(left, bottom), sprite.texture.getUV(3));
		*/
	}

	graphics::mapData(_vertexBuffer, _vertices, _index * 4 * sizeof(SpriteVertex));
	graphics::updateConstantBuffer(_mvpCB, &mvp);
	graphics::setVertexShaderConstantBuffer(_mvpCB);

	graphics::drawIndexed(_index * 6);//  (6, 0);
	_index = 0;
}