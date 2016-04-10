#include "math.h"

namespace math {

	v4 getTextureCoordinates(const Rect& textureRect, float textureWidth, float textureHeight) {
		v4 ret;
		ret.x = textureRect.left / textureWidth;
		ret.z = textureRect.right / textureWidth;
		ret.y = textureRect.top / textureHeight;
		ret.w = textureRect.bottom / textureHeight;
		return ret;
	}

	v4 getTextureCoordinates(float top, float left, float width, float height, float textureWidth, float textureHeight) {
		v4 ret;
		float tw = width / textureWidth;
		float th = height / textureHeight;
		ret.x = left / textureWidth;
		ret.z = ret.x + tw;
		ret.y = top / textureHeight;
		ret.w = ret.y + th;
		return ret;
	}

	Texture buildTexture(float top, float left, float width, float height, float textureWidth, float textureHeight) {
		Texture ret;
		Rect r(top, left, width, height);
		ret.rect = r;
		ret.uv = getTextureCoordinates(r, textureWidth, textureHeight);
		ret.textureID = 0;
		ret.dim = Vector2f(width, height);
		ret.textureSize.x = textureWidth;
		ret.textureSize.y = textureHeight;
		return ret;
	}

	Texture buildTexture(const Rect& r, float textureWidth, float textureHeight, bool useHalfTexel) {
		Texture ret;
		ret.rect = r;
		ret.uv = getTextureCoordinates(r, textureWidth, textureHeight);
		ret.textureID = 0;
		ret.dim = Vector2f(r.width(), r.height());
		ret.textureSize.x = textureWidth;
		ret.textureSize.y = textureHeight;
		return ret;
	}
}