#pragma once
#include "PostProcess.h"
#include "..\renderer\VertexTypes.h"
#include "..\Common.h"

namespace ds {

	struct ScreenShakePostProcessDescriptor {
		float ttl;
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
	private:
		RID _vertexBuffer;
		RID _material;
		RID _cbID;
		PTCVertex _vertices[6];
		ScreenShakePostProcessDescriptor _descriptor;
		ScreenShakeConstantBuffer _constantBuffer;
	};

}

