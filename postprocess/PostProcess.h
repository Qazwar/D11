#pragma once
#include "core\Common.h"

namespace ds {

	struct PostProcessDescriptor {

		RID source;
		RID target;

		PostProcessDescriptor() : source(INVALID_RID), target(INVALID_RID) {}
	};

	class PostProcess {

	public:
		PostProcess(const PostProcessDescriptor& descriptor);
		virtual ~PostProcess() {}
		virtual void init() {}
		void begin();
		void render();
		virtual void tick(float dt) = 0;
		virtual void onActivate() {}
		virtual void onDeactivate() {}
		void activate() {
			_active = true;
			onActivate();
		}
		void deactivate() {
			_active = false;
			onDeactivate();
		}
		virtual void updateConstantBuffer() = 0;
		bool isActive() const {
			return _active;
		}
	protected:
		PostProcess(const PostProcess& other) {}
		bool _active;
		RID _source;
		RID _target;
		RID _vertexBuffer;
		RID _material;
	};
}