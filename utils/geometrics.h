#pragma once
#include "..\renderer\QuadBuffer.h"

namespace ds {

	namespace geometrics {

		void createCube(Mesh* mesh,const Rect& textureRect,const v3& center = v3(0,0,0), const v3& size = v3(1,1,1));

	}
}

