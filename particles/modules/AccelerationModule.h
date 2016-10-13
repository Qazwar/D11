#pragma once
#include "ParticleModule.h"

namespace ds {

	// -------------------------------------------------------
	// Acceleration Module
	// -------------------------------------------------------
	struct AccelerationModuleData : ParticleModuleData {

		float radial;
		float radialVariance;
		v2 acceleration;

		AccelerationModuleData() : radial(0.0f), radialVariance(0.0f), acceleration(0, 0) {}

		void read(const JSONReader& reader, int category) {
			reader.get_float(category, "radial", &radial);
			reader.get_float(category, "radial_variance", &radialVariance);
		}
	};

	class AccelerationModule : public ParticleModule {

	public:
		AccelerationModule() : ParticleModule() {}
		virtual ~AccelerationModule() {}
		void generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end);
		void  update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt);
		const char* getName() const {
			return "acceleration";
		}
		const ParticleModuleType getType() const {
			return PM_ACCELERATION;
		}
		int getDataSize() const {
			return sizeof(v2) * 2;
		}
		void debug(const ParticleModuleData* data, void* buffer, uint32_t count);
	};

}