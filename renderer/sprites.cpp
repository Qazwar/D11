#include "sprites.h"
#include "..\renderer\graphics.h"
#include <assert.h>
#include "core\math\math_types.h"
#include "core\math\matrix.h"
#include "..\resources\ResourceDescriptors.h"
#include "..\resources\ResourceContainer.h"
#include "core\log\Log.h"
#include "core\profiler\Profiler.h"
#include "..\stats\DrawCounter.h"

namespace ds {

	SpriteBuffer::SpriteBuffer(const SpriteBufferDescriptor& descriptor) : _descriptor(descriptor), _index(0), _started(false) {
		// create data
		_maxSprites = descriptor.size;
		_sprites = new Sprite[descriptor.size];
		_vertices = new SpriteVertex[4 * descriptor.size];
		//_screenDimension = v4(graphics::getScreenWidth(), graphics::getScreenHeight(), 1024.0f, 1024.0f);
		_constantBuffer.setScreenSize(v2(graphics::getScreenWidth(), graphics::getScreenHeight()));
		_constantBuffer.setTextureSize(1024.0f, 1024.0f);
	}

	SpriteBuffer::~SpriteBuffer() {
		delete[] _sprites;
		delete[] _vertices;
	}

	void SpriteBuffer::draw(const EntityArray & array) {
	}

	void SpriteBuffer::draw(const Sprite& sprite) {
		draw(sprite.position, sprite.texture, sprite.rotation, sprite.scale, sprite.color);
	}

	void SpriteBuffer::draw(const p2i& position, const ds::Texture& texture, float rotation, const v2& scale, const Color& color, RID material) {
		draw(v2(position.x, position.y), texture, rotation, scale, color, material);
	}

	void SpriteBuffer::draw(const v2& position, const ds::Texture& texture, float rotation, const v2& scale, const Color& color, RID material) {
		if (_started) {
			if (material != INVALID_RID && material != _currentMtrl) {
				flush();
				_currentMtrl = material;
			}
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
		_currentMtrl = _descriptor.material;
	}

	void SpriteBuffer::end() {
		flush();
		_started = false;
	}

	void SpriteBuffer::flush() {
		if (_index > 0) {
			ZoneTracker("SpriteBuffer::flush");
			unsigned int stride = sizeof(SpriteVertex);
			unsigned int offset = 0;
			graphics::turnOffZBuffer();

			graphics::setVertexBuffer(_descriptor.vertexBuffer, &stride, &offset, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
			// FIXME: use material from scene
			//graphics::setMaterial(_descriptor.material);
			graphics::setMaterial(_currentMtrl);
			for (int i = 0; i < _index; ++i) {
				const Sprite& sprite = _sprites[i];
				v4 t;
				t.x = sprite.texture.rect.left;
				t.y = sprite.texture.rect.top;
				t.z = sprite.texture.rect.width();
				t.w = sprite.texture.rect.height();
				_vertices[i] = SpriteVertex(sprite.position, t, v3(sprite.scale.x, sprite.scale.y, sprite.rotation), sprite.color);
			}
			graphics::mapData(_descriptor.vertexBuffer, _vertices, _index * sizeof(SpriteVertex));
			mat4 w = matrix::m4identity();
			_constantBuffer.wvp = ds::matrix::mat4Transpose(w * graphics::getOrthoCamera()->getViewProjectionMatrix());
			graphics::updateSpriteConstantBuffer(_constantBuffer);
			graphics::draw(_index);
			graphics::turnOnZBuffer();
			gDrawCounter->sprites += _index;
			_index = 0;
		}
	}

	void SpriteBuffer::drawScreenQuad(RID material) {
		ZoneTracker("SpriteBuffer::drawScreenQuad");
		// if something is still pending
		flush();
		// draw quad
		unsigned int stride = sizeof(SpriteVertex);
		unsigned int offset = 0;
		graphics::turnOffZBuffer();
		SpriteVertex sp(v2(512, 384), v4(0.0f, 0.0f, 1024.0f, 768.0f), v3(1.0f, 1.0f, 0.0f), Color::WHITE);
		graphics::setVertexBuffer(_descriptor.vertexBuffer, &stride, &offset, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		// FIXME: use material from scene
		graphics::setMaterial(material);
		graphics::mapData(_descriptor.vertexBuffer, &sp, sizeof(SpriteVertex));
		_constantBuffer.wvp = ds::matrix::mat4Transpose(graphics::getOrthoCamera()->getViewProjectionMatrix());
		graphics::updateSpriteConstantBuffer(_constantBuffer);
		graphics::draw(1);
		graphics::turnOnZBuffer();
		gDrawCounter->sprites += _index;
	}

}