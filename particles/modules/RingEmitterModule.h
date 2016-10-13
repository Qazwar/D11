#pragma once
#include "ParticleModule.h"

namespace ds {

	// -------------------------------------------------------
	// Ring Location Module
	// -------------------------------------------------------
	struct RingEmitterModuleData : ParticleModuleData {

		float radius;
		float variance;
		float angleVariance;
		float step;

		RingEmitterModuleData() : ParticleModuleData(), radius(10.0f), variance(0.0f), angleVariance(0.0f), step(0.0f) {}

		void read(const JSONReader& reader, int category) {
			reader.get_float(category, "radius", &radius);
			reader.get_float(category, "variance", &variance);
			reader.get_float(category, "angle_variance", &angleVariance);
			reader.get_float(category, "step", &step);
		}
	};

	class RingEmitterModule : public ParticleModule {

	public:
		RingEmitterModule() : ParticleModule() {
			m_Angle = 0.0f;
		}
		virtual ~RingEmitterModule() {}
		void generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end);
		void  update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt) {}
		const char* getName() const {
			return "ring_location";
		}
		const ParticleModuleType getType() const {
			return PM_RING;
		}
		int getDataSize() const {
			return sizeof(float);
		}
		void debug(const ParticleModuleData* data, void* buffer, uint32_t count) {

		}
	private:
		float m_Angle;
	};

}