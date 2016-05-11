#pragma once
#include <stdint.h>
#include <Vector.h>
#include "..\math\math_types.h"

namespace ds {
	
	// -------------------------------------------------------
	// Texture
	// -------------------------------------------------------
	struct TextureAsset {
		//IdString name;
		//LPDIRECT3DTEXTURE9 texture;
		uint32_t width;
		uint32_t height;
	};

	struct Texture {

		//IdString hashName;
		v4 uv;
		v2 dim;
		int textureID;
		Rect rect;
		v2 textureSize;

		Texture() : uv(0, 0, 1, 1), dim(32, 32), textureID(0), rect(0, 0, 32, 32), textureSize(1024.0f, 1024.0f) {}

		const v2 getUV(int idx) const {
			switch (idx) {
			case 0: return v2(uv.x, uv.y); break;
			case 1: return v2(uv.z, uv.y); break;
			case 2: return v2(uv.z, uv.w); break;
			case 3: return v2(uv.x, uv.w); break;
			default: return v2(0, 0);
			}
		}
	};

	struct Plane {

		v3 position;
		v2 size;
		Texture texture;


	};

	

}