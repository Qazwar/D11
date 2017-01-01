#pragma once
#include <core\base\StateObject.h>
#include <core\Common.h>

namespace ds {

	struct EffectSettings {
		RID sourceRT;
		RID targetRT;
	};

	class BaseEffect : public StateObject {

	public:
		BaseEffect(EffectSettings* settings) : _settings(settings) {}
		virtual ~BaseEffect() {}
		virtual void tick(float dt) = 0;
		virtual void begin() = 0;
		virtual void end() = 0;
		void virtual showDialog() {}
		void virtual saveReport() {}
	protected:
		EffectSettings* _settings;
	};

	class FullScreenEffect : public BaseEffect {

	public:
		virtual void updateConstantBuffer() = 0;
		void end();
	};

}
