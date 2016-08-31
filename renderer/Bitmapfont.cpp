#include "Bitmapfont.h"
#include "core\math\math.h"

namespace ds {

	Bitmapfont::Bitmapfont() {
	}


	Bitmapfont::~Bitmapfont() {
	}

	void Bitmapfont::add(int ascii, const Rect& r,float xOffset, float yOffset,float textureSize) {
		Rect nr = r;
		nr.top += yOffset;
		nr.bottom += yOffset;
		nr.left += xOffset;
		nr.right += xOffset;
		_textures[ascii] = math::buildTexture(nr, textureSize, textureSize);
	}

}