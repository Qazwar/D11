#pragma once
#include <stdint.h>
#include <Point.h>
#include "render_types.h"
#include "core\graphics\Color.h"
#include "graphics.h"
#include "..\resources\ResourceDescriptors.h"
#include "..\sprites\Sprite.h"
#include "VertexTypes.h"
#include "..\scene\EntityArray.h"

namespace ds {

	class SpriteBuffer {

	public:
		SpriteBuffer(const SpriteBufferDescriptor& descriptor);
		~SpriteBuffer();
		void draw(const EntityArray& array);
		void draw(const v2& position, const ds::Texture& texture, float rotation = 0.0f, const v2& scale = v2(1, 1), const Color& color = Color(255, 255, 255, 255), RID material = INVALID_RID);
		void draw(const p2i& position, const ds::Texture& texture, float rotation = 0.0f, const v2& scale = v2(1, 1), const Color& color = Color(255, 255, 255, 255), RID material = INVALID_RID);
		void draw(const Sprite& sprite);
		void drawText(RID fontID, int x, int y, const char* text, int padding = 4, float scaleX = 1.0f, float scaleY = 1.0f, const Color& color = Color(255, 255, 255, 255));
		void drawTiledX(const v2& position, float width, const Texture& texture, float cornersize, const Color& color = Color::WHITE);
		void drawTiledXY(const v2& position, const v2& size, const Texture& texture, float cornersize, const Color& color = Color::WHITE);
		void drawLine(const v2& start, const v2& end, const ds::Texture& texture, const Color& color = Color(255, 255, 255, 255), RID material = INVALID_RID);
		void begin();
		void end();

		void drawScreenQuad(RID material);
		RID getCurrentMaterial() const {
			return _currentMtrl;
		}
		void setMaterial(RID mtrl) {
			if (mtrl != _currentMtrl) {
				flush();
				_currentMtrl = mtrl;
			}
		}
	private:
		void flush();
		int _index;
		RID _currentMtrl;
		SpriteBufferDescriptor _descriptor;
		Sprite* _sprites;
		SpriteVertex* _vertices;
		int _maxSprites;
		bool _started;
		//v4 _screenDimension;
		SpriteBufferCB _constantBuffer;
	};

}