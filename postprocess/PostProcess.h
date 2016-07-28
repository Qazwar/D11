#pragma once

namespace ds {

	class PostProcess {

	public:
		PostProcess() : _active(false) {}
		virtual ~PostProcess() {}
		virtual void init() {}
		virtual void render() = 0;
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
	protected:
		bool _active;
	};
}