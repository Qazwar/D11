#pragma once
#include "PostProcess.h"

namespace ds {

	class SimplePostProcess : public PostProcess {

	public:
		void render() {}
		void tick(float dt) {}
	};

}