#pragma once
#include <Vector.h>
#include "core\graphics\Color.h"

namespace ds {

	struct QuadVertex {
		v3 position;
		v2 texture;
		Color color;

		QuadVertex() : position(0, 0, 0) {}
		QuadVertex(const v3& p, const v2& t, const Color& c) : position(p), texture(t), color(c) {}
		QuadVertex(const v2& p, const v2& t, const Color& c) : position(p, 1.0f), texture(t), color(c) {}
	};

	struct PTCVertex {
		v3 position;
		v2 texture;
		Color color;

		PTCVertex() : position(0, 0, 0) , texture(0,0), color(Color::WHITE) {}
		PTCVertex(const v3& p, const v2& t, const Color& c) : position(p), texture(t), color(c) {}
	};

	struct PNTCVertex {
		v3 position;
		v3 normal;
		v2 texture;
		Color color;

		PNTCVertex() : position(0, 0, 0) {}
		PNTCVertex(const v3& p, const v2& t, const Color& c) : position(p), texture(t), color(c) {}
		PNTCVertex(const v3& p, const v3& n, const v2& t, const Color& c) : position(p), normal(n), texture(t), color(c) {}
		PNTCVertex(const v2& p, const v2& t, const Color& c) : position(p, 1.0f), texture(t), color(c) {}
	};

	struct SpriteVertex {

		v3 position;
		v4 texture;
		v3 size;
		Color color;

		SpriteVertex() : position(0, 0, 0) {}
		SpriteVertex(const v3& p, const v4& t, const Color& c) : position(p), texture(t), color(c) {}
		SpriteVertex(const v2& p, const v4& t, const Color& c) : position(p, 1.0f), texture(t), color(c) {}
		SpriteVertex(const v2& p, const v4& t, const v3& s,const Color& c) : position(p, 1.0f), texture(t), size(s), color(c) {}
	};

}