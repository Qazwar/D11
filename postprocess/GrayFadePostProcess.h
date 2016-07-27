#pragma once
#include "PostProcess.h"
#include "..\renderer\VertexTypes.h"
#include "..\Common.h"

namespace ds {

	struct GrayFadePostProcessDescriptor {
		float ttl;
	};

	class GrayFadePostProcess : public PostProcess {

	public:
		GrayFadePostProcess(const GrayFadePostProcessDescriptor& descriptor);
		virtual ~GrayFadePostProcess();
		void render();
		void tick(float dt);
	private:
		float _timer;
		RID _vertexBuffer;
		RID _material;
		RID _cbID;
		PTCVertex _vertices[6];
		GrayFadePostProcessDescriptor _descriptor;
	};

}

