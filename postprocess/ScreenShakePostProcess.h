#pragma once
#include "PostProcess.h"
#include "..\renderer\VertexTypes.h"
#include "..\Common.h"

namespace ds {

	struct ScreenShakePostProcessDescriptor {
		float ttl;
	};

	class ScreenShakePostProcess : public PostProcess {

	public:
		ScreenShakePostProcess(const ScreenShakePostProcessDescriptor& descriptor);
		virtual ~ScreenShakePostProcess();
		void render();
		void tick(float dt);
		void onActivate() {
			_timer = 0.0f;
		}
	private:
		float _timer;
		v4 _cbData;
		RID _vertexBuffer;
		RID _material;
		RID _cbID;
		PTCVertex _vertices[6];
		ScreenShakePostProcessDescriptor _descriptor;
	};

}

