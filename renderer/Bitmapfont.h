#pragma once
#include "render_types.h"
#include "..\math\math_types.h"

namespace ds {

	class Bitmapfont {

	public:
		Bitmapfont();
		~Bitmapfont();
		void add(int ascii, const Rect& r);
		const Texture& get(int ascii) const {
			return _textures[ascii];
		}
	private:
		Texture _textures[256];
	};

}

