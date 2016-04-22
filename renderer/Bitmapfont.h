#pragma once
#include "render_types.h"
#include "..\math\math_types.h"

namespace ds {

	class Bitmapfont {

	public:
		Bitmapfont();
		~Bitmapfont();
		void add(int ascii, const Rect& r, float xOffset = 0.0f, float yOffset = 0.0f, float textureSize = 1024.0f);
		const Texture& get(int ascii) const {
			return _textures[ascii];
		}
	private:
		Texture _textures[256];
	};

}

