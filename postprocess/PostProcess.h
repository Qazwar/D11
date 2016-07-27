#pragma once

namespace ds {

	class PostProcess {

	public:
		PostProcess() : _active(false) {}
		virtual ~PostProcess() {}
		virtual void init() {}
		virtual void render() = 0;
		virtual void tick(float dt) = 0;
	protected:
		bool _active;
	};
}