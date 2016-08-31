#pragma once
#include <stdint.h>
#include <Vector.h>
#include "core\math\math_types.h"
#include "core\Common.h"
#include "core\math\matrix.h"
#include "core\graphics\Color.h"

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

}