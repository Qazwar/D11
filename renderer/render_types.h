#pragma once
#include <stdint.h>
#include <Vector.h>
#include "..\math\math_types.h"
#include "..\utils\Color.h"
#include "..\Common.h"

namespace ds {
	
	struct Material {
		Color diffuse;
		Color ambient;
		RID blendState;
		RID shader;
		RID texture;
	};

	struct Texture {

		//IdString hashName;
		//v4 uv;
		v2 uv[4];
		v2 dim;
		int textureID;
		Rect rect;
		v2 textureSize;

		Texture() : dim(32, 32), textureID(0), rect(0, 0, 32, 32), textureSize(1024.0f, 1024.0f) {
			uv[0] = v2(0, 0);
			uv[1] = v2(1, 0);
			uv[2] = v2(1, 1);
			uv[3] = v2(0, 1);
		}
		/*
		const v2 getUV(int idx) const {
			switch (idx) {
			case 0: return v2(uv.x, uv.y); break;
			case 1: return v2(uv.z, uv.y); break;
			case 2: return v2(uv.z, uv.w); break;
			case 3: return v2(uv.x, uv.w); break;
			default: return v2(0, 0);
			}
		}
		*/
	};
	
}