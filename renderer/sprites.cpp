#include "sprites.h"
#include "..\graphics.h"
#include <assert.h>
#include "renderQueue.h"
#include "..\math\math_types.h"
#include "..\math\matrix.h"
#include "..\resources\ResourceDescriptors.h"
#include "..\utils\Log.h"
#include "..\utils\Profiler.h"

namespace ds {

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

	SpriteBuffer::SpriteBuffer(const SpriteBufferDescriptor& descriptor) : _descriptor(descriptor), _index(0), _started(false) {
		// create data
		_maxSprites = descriptor.size;
		_sprites = new Sprite[descriptor.size];
		_vertices = new SpriteVertex[4 * descriptor.size];
	}

	SpriteBuffer::~SpriteBuffer() {
	}

	void SpriteBuffer::draw(const Sprite& sprite) {
		draw(sprite.position, sprite.texture, sprite.rotation, sprite.scale, sprite.color);
	}

	void SpriteBuffer::draw(const v2& position, const ds::Texture& texture, float rotation, const v2& scale, const Color& color) {
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

	void SpriteBuffer::drawText(RID fontID, int x, int y, const char* text, int padding, float scaleX, float scaleY, const Color& color) {
		ds::Bitmapfont* font = graphics::getFont(fontID);
		int len = strlen(text);
		for (int cnt = 0; cnt < len; ++cnt) {
			char c = text[cnt];
			const ds::Texture& t = font->get(c);
			float dimX = t.dim.x * scaleX;
			float dimY = t.dim.y * scaleY;
			draw(v2(x + dimX * 0.5f, y + dimY * 0.5f), t, 0.0f, v2(scaleX, scaleY), color);
			x += dimX + padding;
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
		ZoneTracker("SpriteBuffer::flush");
		unsigned int stride = sizeof(SpriteVertex);
		unsigned int offset = 0;

		graphics::setInputLayout(_descriptor.inputlayout);
		graphics::setVertexBuffer(_descriptor.vertexBuffer, &stride, &offset);
		graphics::setIndexBuffer(_descriptor.indexBuffer);
		graphics::setBlendState(_descriptor.blendstate);

		graphics::setShader(_descriptor.shader);
		graphics::setPixelShaderResourceView(_descriptor.colormap);

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

		graphics::mapData(_descriptor.vertexBuffer, _vertices, _index * 4 * sizeof(SpriteVertex));

		graphics::updateConstantBuffer(_descriptor.constantBuffer, &mvp);
		graphics::setVertexShaderConstantBuffer(_descriptor.constantBuffer);

		graphics::drawIndexed(_index * 6);

		_index = 0;
	}

}