#pragma once
#include "PostProcess.h"
#include "..\renderer\VertexTypes.h"
#include "..\Common.h"

namespace ds {

	struct ScreenShakePostProcessDescriptor : public PostProcessDescriptor {

		float ttl;
		float shakeAmount;
		float frequency;

		ScreenShakePostProcessDescriptor() : PostProcessDescriptor(), ttl(1.0f) , shakeAmount(2.0f) , frequency(1.0f) {}
	};

	class ScreenShakePostProcess : public PostProcess {

		struct ScreenShakeConstantBuffer {
			float timer;
			float period;
			float shakeAmount;
			float frequency;

			ScreenShakeConstantBuffer() : timer(0.0f), period(1.0f), shakeAmount(0.0f), frequency(0.0f) {}

		};

	public:
		ScreenShakePostProcess(const ScreenShakePostProcessDescriptor& descriptor);
		virtual ~ScreenShakePostProcess();
		void render();
		void tick(float dt);
		void onActivate() {
			_constantBuffer.timer = 0.0f;
		}
		void updateConstantBuffer();
	private:
		RID _cbID;
		ScreenShakePostProcessDescriptor _descriptor;
		ScreenShakeConstantBuffer _constantBuffer;
	};

}

