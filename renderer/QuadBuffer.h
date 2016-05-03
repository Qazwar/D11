#pragma once
#include <stdint.h>
#include "render_types.h"
#include "..\utils\Color.h"
#include "graphics.h"
#include "..\resources\ResourceDescriptors.h"
#include "..\sprites\Sprite.h"

namespace ds {

	class QuadBuffer {

		struct QuadVertex {
			v3 position;
			v2 texture;
			Color color;

			QuadVertex() : position(0, 0, 0) {}
			QuadVertex(const v3& p, const v2& t, const Color& c) : position(p), texture(t), color(c) {}
			QuadVertex(const v2& p, const v2& t, const Color& c) : position(p, 1.0f), texture(t), color(c) {}
		};

	public:
		QuadBuffer(const QuadBufferDescriptor& descriptor);
		~QuadBuffer();
		void draw(const v3& position, const v2& uv, const Color& color = Color(255, 255, 255, 255));
		void begin();
		void end();
	private:
		void flush();
		int _index;
		QuadBufferDescriptor _descriptor;
		QuadVertex* _vertices;
		int _maxSprites;
		bool _started;
	};

}

