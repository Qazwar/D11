#include "sprites.h"
#include "..\renderer\graphics.h"
#include <assert.h>
#include "renderQueue.h"
#include "..\math\math_types.h"
#include "..\math\matrix.h"
#include "..\resources\ResourceDescriptors.h"
#include "..\resources\ResourceContainer.h"
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
		delete[] _sprites;
		delete[] _vertices;
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
		ds::Bitmapfont* font = ds::res::getFont(fontID);
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

	void SpriteBuffer::drawTiledXY(const v2& position, const v2& size, const Texture& texture, float cornersize, const Color& color) {
		v2 center = position;
		center.x += size.x * 0.5f;
		center.y -= size.y * 0.5f;
		float ch = size.y - 2.0f * cornersize;
		float cw = size.x - 2.0f * cornersize;
		float hcz = cornersize * 0.5f;
		float sx = (size.x - 2.0f * cornersize) / (texture.dim.x - 2.0f * cornersize);
		float sy = (size.y - 2.0f * cornersize) / (texture.dim.y - 2.0f * cornersize);
		// left top corner 
		v2 p = center;
		ds::Texture tex = math::buildTexture(texture.rect.top, texture.rect.left, cornersize, cornersize, texture.textureSize.x, texture.textureSize.y);
		p.x = center.x - cw * 0.5f - hcz;
		p.y = center.y + ch * 0.5f + hcz;
		draw(p, tex, 0.0f, v2(1.0f, 1.0f), color);
		// right top corner
		tex = math::buildTexture(texture.rect.top, texture.rect.right - cornersize, cornersize, cornersize, texture.textureSize.x, texture.textureSize.y);
		p.x = center.x + cw * 0.5f + hcz;
		p.y = center.y + ch * 0.5f + hcz;
		draw(p, tex, 0.0f, v2(1.0f, 1.0f), color);
		// left bottom corner
		tex = math::buildTexture(texture.rect.bottom - cornersize, texture.rect.left, cornersize, cornersize, texture.textureSize.x, texture.textureSize.y);
		p.x = center.x - cw * 0.5f - hcz;
		p.y = center.y - ch * 0.5f - hcz;
		draw(p, tex, 0.0f, v2(1.0f, 1.0f), color);
		// right bottom corner
		tex = math::buildTexture(texture.rect.bottom - cornersize, texture.rect.right - cornersize, cornersize, cornersize, texture.textureSize.x, texture.textureSize.y);
		p.x = center.x + cw * 0.5f + hcz;
		p.y = center.y - ch * 0.5f - hcz;
		draw(p, tex, 0.0f, v2(1.0f, 1.0f), color);
		// top
		tex = math::buildTexture(texture.rect.top, texture.rect.left + cornersize, texture.rect.width() - 2.0f * cornersize, cornersize, texture.textureSize.x, texture.textureSize.y);
		p.x = center.x;
		p.y = center.y + ch * 0.5f + hcz;
		draw(p, tex, 0.0f, v2(sx, 1.0f), color);
		// bottom
		tex = math::buildTexture(texture.rect.bottom - cornersize, texture.rect.left + cornersize, texture.rect.width() - 2.0f * cornersize, cornersize, texture.textureSize.x, texture.textureSize.y);
		p.x = center.x;
		p.y = center.y - ch * 0.5f - hcz;
		draw(p, tex, 0.0f, v2(sx, 1.0f), color);
		// left
		tex = math::buildTexture(texture.rect.top + cornersize, texture.rect.left, cornersize, texture.rect.height() - 2.0f *cornersize, texture.textureSize.x, texture.textureSize.y);
		p.x = center.x - cw * 0.5f - hcz;
		p.y = center.y;
		draw(p, tex, 0.0f, v2(1.0f, sy), color);
		// right
		tex = math::buildTexture(texture.rect.top + cornersize, texture.rect.right - cornersize, cornersize, texture.rect.height() - 2.0f *cornersize, texture.textureSize.x, texture.textureSize.y);
		p.x = center.x + cw * 0.5f + hcz;
		p.y = center.y;
		draw(p, tex, 0.0f, v2(1.0f, sy), color);
		// center
		tex = math::buildTexture(texture.rect.top + cornersize, texture.rect.left + cornersize, texture.rect.width() - 2.0f * cornersize, texture.rect.height() - 2.0f * cornersize, texture.textureSize.x, texture.textureSize.y);
		p.x = center.x;
		p.y = center.y;
		draw(p, tex, 0.0f, v2(sx, sy), color);

	}

	void SpriteBuffer::drawTiledX(const v2& position, float width, const Texture& texture, float cornersize, const Color& color) {
		v2 center = position;
		center.x += width * 0.5f;
		float cw = width - 2.0f * cornersize;
		float hcz = cornersize * 0.5f;
		float sx = (width - 2.0f * cornersize) / (texture.dim.x - 2.0f * cornersize);
		// left 
		v2 p = center;
		ds::Texture tex = math::buildTexture(texture.rect.top, texture.rect.left, cornersize, cornersize, texture.textureSize.x, texture.textureSize.y);
		p.x = center.x - cw * 0.5f - hcz;
		draw(p, tex, 0.0f,v2(1.0f, 1.0f), color);
		// right
		tex = math::buildTexture(texture.rect.top, texture.rect.right - cornersize, cornersize, cornersize, texture.textureSize.x, texture.textureSize.y);
		p.x = center.x + cw * 0.5f + hcz;
		draw(p, tex, 0.0f, v2(1.0f, 1.0f), color);
		// center
		tex = math::buildTexture(texture.rect.top, texture.rect.left + cornersize, texture.rect.width() - 2.0f * cornersize, texture.rect.height(), texture.textureSize.x, texture.textureSize.y);
		p.x = center.x;
		p.y = center.y;
		draw(p, tex, 0.0f, v2(sx, 1.0f), color);

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

		//ds::mat4 mvp = graphics::getViewProjectionMaxtrix();
		ds::mat4 mvp = graphics::getCamera()->getViewProjectionMatrix();
		mvp = ds::matrix::mat4Transpose(mvp);

		v2 sc = graphics::getScreenCenter();
		for (int i = 0; i < _index; i++) {
			const Sprite& sprite = _sprites[i];
			for (int j = 0; j < 4; ++j) {
				v2 start = ARRAY[j];
				start.x *= sprite.texture.dim.x;
				start.y *= sprite.texture.dim.y;
				v2 sp = sprite.position;
				sp -= sc;
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