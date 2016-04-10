#pragma once
#include <stdint.h>
#include <Vector.h>

// -------------------------------------------------------
// Rect
// -------------------------------------------------------
struct Rect {

	float top, left, bottom, right;
	Rect();
	Rect(const Rect& rect);
	Rect(float top, float left, float width, float height);
	Rect(int _top, int _left, int _width, int _height);
	const float width() const;
	const float height() const;

	bool isInside(float x, float y) {
		if (x > right) {
			return false;
		}
		if (x < left) {
			return false;
		}
		if (y < bottom) {
			return false;
		}
		if (y > top) {
			return false;
		}
		return true;
	}
};

inline Rect::Rect() : top(0.0f), left(0.0f), bottom(0.0f), right(0.0f) {}

inline Rect::Rect(float _top, float _left, float width, float height) : top(_top), left(_left) {
	bottom = _top + height;
	right = _left + width;
}

inline Rect::Rect(int _top, int _left, int _width, int _height) {
	top = static_cast<float>(_top);
	left = static_cast<float>(_left);
	bottom = top + static_cast<float>(_height);
	right = left + static_cast<float>(_width);
}

inline Rect::Rect(const Rect& rect) {
	top = rect.top;
	left = rect.left;
	bottom = rect.bottom;
	right = rect.right;
}

inline const float Rect::width() const {
	return right - left;
}

inline const float Rect::height() const {
	return bottom - top;
}

// -------------------------------------------------------
// Texture
// -------------------------------------------------------
struct TextureAsset {
	//IdString name;
	//LPDIRECT3DTEXTURE9 texture;
	uint32_t width;
	uint32_t height;
};

struct Texture {

	//IdString hashName;
	v4 uv;
	v2 dim;
	int textureID;
	Rect rect;
	v2 textureSize;

	Texture() : uv(0, 0, 1, 1), dim(32, 32), textureID(0) , rect(0,0,32,32) , textureSize(1024.0f,1024.0f) {}

	const v2 getUV(int idx) const {
		switch (idx) {
			case 0: return v2(uv.x, uv.y); break;
			case 1: return v2(uv.z, uv.y); break;
			case 2: return v2(uv.z, uv.w); break;
			case 3: return v2(uv.x, uv.w); break;
			default: return v2(0, 0);
		}
	}
};

