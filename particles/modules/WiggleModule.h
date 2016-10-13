#pragma once
#include "ParticleModule.h"

namespace ds {

	// -------------------------------------------------------
	// Acceleration Module
	// -------------------------------------------------------
	struct WiggleModuleData : ParticleModuleData {

		float frequency;
		float frequencyVariance;
		float amplitude;
		float amplitudeVariance;

		WiggleModuleData() : frequency(0.0f), amplitude(0.0f) , frequencyVariance(0.0f) , amplitudeVariance(0.0f) {}

		void read(const JSONReader& reader, int category) {
			reader.get_float(category, "frequency", &frequency);
			reader.get_float(category, "amplitude", &amplitude);
			reader.get_float(category, "frequency_variance", &frequencyVariance);
			reader.get_float(category, "amplitude_variance", &amplitudeVariance);
		}
	};

	class WiggleModule : public ParticleModule {

	public:
		WiggleModule() : ParticleModule() {}
		virtual ~WiggleModule() {}
		void generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end);
		void  update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt);
		const char* getName() const {
			return "wiggle";
		}
		const ParticleModuleType getType() const {
			return PM_WIGGLE;
		}
		int getDataSize() const {
			return sizeof(v2);
		}
		void debug(const ParticleModuleData* data, void* buffer, uint32_t count);
	};

}