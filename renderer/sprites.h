#pragma once
#include <stdint.h>
#include "render_types.h"
#include "..\utils\Color.h"
#include "..\graphics.h"
#include "..\resources\ResourceDescriptors.h"

namespace ds {

	struct Sprite {
		v2 position;
		float rotation;
		v2 scale;
		ds::Texture texture;
		Color color;
	};

	class SpriteBuffer {

		struct SpriteVertex {
			v3 position;
			v2 texture;
			Color color;

			SpriteVertex() : position(0, 0, 0) {}
			SpriteVertex(const v3& p, const v2& t, const Color& c) : position(p), texture(t), color(c) {}
			SpriteVertex(const v2& p, const v2& t, const Color& c) : position(p, 1.0f), texture(t), color(c) {}
		};

	public:
		SpriteBuffer(const SpriteBufferDescriptor& descriptor);
		~SpriteBuffer();
		void draw(const v2& position, const ds::Texture& texture, float rotation = 0.0f, const v2& scale = v2(1, 1), const Color& color = Color(255, 255, 255, 255));
		void draw(const Sprite& sprite);
		void drawText(RID fontID, int x, int y, const char* text, int padding = 4, float scaleX = 1.0f, float scaleY = 1.0f, const Color& color = Color(255, 255, 255, 255));
		void begin();
		void end();
	private:
		void flush();
		int _index;
		SpriteBufferDescriptor _descriptor;
		Sprite* _sprites;
		SpriteVertex* _vertices;
		int _maxSprites;
		bool _started;
	};

}