#include "Bitmapfont.h"
#include "..\math\math.h"

namespace ds {

	Bitmapfont::Bitmapfont()
	{
	}


	Bitmapfont::~Bitmapfont()
	{
	}

	void Bitmapfont::add(int ascii, const Rect& r) {
		_textures[ascii] = math::buildTexture(r);
	}

}