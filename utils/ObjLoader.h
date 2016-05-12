#pragma once
#include "..\renderer\MeshBuffer.h"

namespace ds {

	namespace obj {

		bool parse(const char* fileName, Mesh* mesh,const v3& offset = v3(0,0,0), const v3& scale = v3(1,1,1), const v3& rotation = v3(0,0,0));

	}
}