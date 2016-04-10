#include "sprites.h"
#include "..\graphics.h"

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
	// create vertexbuffer
	// create blendstate
	// load texture
	// load shader
	_shaderIndex = graphics::compileShader("TextureMap.fx");

	// create layout
	D3D11_INPUT_ELEMENT_DESC spriteLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	_layoutIndex = graphics::createInputLayout(_shaderIndex, spriteLayout, 2);
}

void SpriteBuffer::draw(const v2& position, const Texture& texture) {
	if (_started) {

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

}