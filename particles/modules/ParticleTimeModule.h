#pragma once
#include "ParticleModule.h"

namespace ds {
	// -------------------------------------------------------
	// LifetimeModuleData
	// -------------------------------------------------------
	struct LifetimeModuleData : ParticleModuleData {

		float ttl;
		float variance;

		LifetimeModuleData() : ParticleModuleData(), ttl(1.0f), variance(0.0f) {}

		void read(const JSONReader& reader, int category) {
			reader.get_float(category, "ttl", &ttl);
			reader.get_float(category, "variance", &variance);
		}


	};

	// -------------------------------------------------------
	// ParticleTimeModule
	// -------------------------------------------------------
	class ParticleTimeModule : public ParticleModule {

	public:
		ParticleTimeModule() : ParticleModule() {}
		virtual ~ParticleTimeModule() {}
		void generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end);
		void  update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt);
		const ParticleModuleType getType() const {
			return PM_LIFECYCLE;
		}
		const char* getName() const {
			return "lifecycle";
		}

		int getDataSize() const {
			return sizeof(float);
		}
		void debug(const ParticleModuleData* data, void* buffer, uint32_t count);
	};

}