#pragma once
#include "ParticleModule.h"

namespace ds {

	// -------------------------------------------------------
	// Rotation Velocity Module
	// -------------------------------------------------------
	struct RotationModuleData : ParticleModuleData {

		v2 velocityRange;

		RotationModuleData() : velocityRange(0.0f) {}

		void read(const JSONReader& reader, int category) {
			if (reader.contains_property(category, "velocity_range")) {
				reader.get_vec2(category, "velocity_range", &velocityRange);
				velocityRange.x = DEGTORAD(velocityRange.x);
				velocityRange.y = DEGTORAD(velocityRange.y);
			}
			else {
				float velocity;
				float variance;
				reader.get_float(category, "velocity", &velocity);
				reader.get_float(category, "variance", &variance);
				velocityRange.x = DEGTORAD(velocity - variance);
				velocityRange.y = DEGTORAD(velocity + variance);
			}
		}
	};

	class RotationModule : public ParticleModule {

	public:
		RotationModule() : ParticleModule() {}
		virtual ~RotationModule() {}
		void generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end);
		void  update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt);
		const char* getName() const {
			return "rotation";
		}
		const ParticleModuleType getType() const {
			return PM_ROTATION;
		}
		int getDataSize() const {
			return sizeof(float);
		}
		void debug(const ParticleModuleData* data, void* buffer, uint32_t count) {

		}
	};

}