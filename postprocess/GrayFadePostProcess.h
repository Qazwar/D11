#pragma once
#include "PostProcess.h"
#include "..\renderer\VertexTypes.h"
#include "..\Common.h"

namespace ds {

	struct GrayFadePostProcessDescriptor : public PostProcessDescriptor {

		float ttl;

		GrayFadePostProcessDescriptor() : PostProcessDescriptor() , ttl(1.0f) {}
	};

	class GrayFadePostProcess : public PostProcess {

	public:
		GrayFadePostProcess(const GrayFadePostProcessDescriptor& descriptor);
		virtual ~GrayFadePostProcess();
		void render();
		void tick(float dt);
		void onActivate() {
			_timer = 0.0f;
		}
		void updateConstantBuffer();
	private:
		float _timer;
		v4 _cbData;
		RID _cbID;
		GrayFadePostProcessDescriptor _descriptor;
	};

}

