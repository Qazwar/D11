#pragma once

#include "..\renderer\render_types.h"

namespace math {

	ds::Texture buildTexture(float top, float left, float width, float height, float textureWidth = 1024.0f, float textureHeight = 1024.0f);

	ds::Texture buildTexture(const ds::Rect& r, float textureWidth = 1024.0f, float textureHeight = 1024.0f);
}