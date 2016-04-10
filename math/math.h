#pragma once

#include "..\renderer\render_types.h"

namespace math {

	Texture buildTexture(float top, float left, float width, float height, float textureWidth = 1024.0f, float textureHeight = 1024.0f);

	Texture buildTexture(const Rect& r, float textureWidth = 1024.0f, float textureHeight = 1024.0f);
}