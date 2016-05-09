#pragma once
#include <Vector.h>
#include "..\utils\Color.h"

namespace ds {

	struct QuadVertex {
		v3 position;
		v2 texture;
		Color color;

		QuadVertex() : position(0, 0, 0) {}
		QuadVertex(const v3& p, const v2& t, const Color& c) : position(p), texture(t), color(c) {}
		QuadVertex(const v2& p, const v2& t, const Color& c) : position(p, 1.0f), texture(t), color(c) {}
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
		v2 texture;
		Color color;

		SpriteVertex() : position(0, 0, 0) {}
		SpriteVertex(const v3& p, const v2& t, const Color& c) : position(p), texture(t), color(c) {}
		SpriteVertex(const v2& p, const v2& t, const Color& c) : position(p, 1.0f), texture(t), color(c) {}
	};

}