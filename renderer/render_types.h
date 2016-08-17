#pragma once
#include <stdint.h>
#include <Vector.h>
#include "..\math\math_types.h"
#include "..\utils\Color.h"
#include "..\Common.h"
#include "..\math\matrix.h"

namespace ds {
	
	struct Material {
		Color diffuse;
		Color ambient;
		RID blendState;
		RID shader;
		RID texture;
		RID renderTarget;
	};

	enum DrawMode {
		STATIC,
		TRANSFORM,
		IMMEDIATE
	};

	struct SpriteBufferCB {

		v4 screenDimension;
		v4 screenCenter;
		mat4 wvp;

		SpriteBufferCB() : screenDimension(v4(1024, 768, 1024, 1024)) , screenCenter(v4(512,384,0,0)) {}

		void setScreenSize(const v2& scrSize) {
			screenDimension.x = scrSize.x;
			screenDimension.y = scrSize.y;
			screenCenter.x = screenDimension.x * 0.5f;
			screenCenter.y = screenDimension.y * 0.5f;
		}

		void setTextureSize(float w, float h) {
			screenDimension.z = w;
			screenDimension.w = h;
		}
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

		explicit Texture(const Rect& r) {
			build(r);
		}

		Texture(int top, int left, int width, int height) {
			build(Rect(top, left, width, height));
		}

		void build(const Rect& r) {
			rect = r;
			v4 cuv;
			cuv.x = r.left / textureSize.x;
			cuv.z = r.right / textureSize.x;
			cuv.y = r.top / textureSize.y;
			cuv.w = r.bottom / textureSize.y;
			uv[0] = v2(cuv.x, cuv.y);
			uv[1] = v2(cuv.z, cuv.y);
			uv[2] = v2(cuv.z, cuv.w);
			uv[3] = v2(cuv.x, cuv.w);
			textureID = 0;
			dim = v2(r.width(), r.height());
		}

		void move(int dx, int dy) {
			rect.top += dy;
			rect.bottom += dy;
			rect.left += dx;
			rect.right += dx;
			build(rect);
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