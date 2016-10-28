#pragma once
#include "ParticleModule.h"

namespace ds {

	// -------------------------------------------------------
	// Point Location Module
	// -------------------------------------------------------
	struct PointEmitterModuleData : ParticleModuleData {

		float rotation;

		PointEmitterModuleData() : ParticleModuleData(), rotation(0.0f) {}

		void read(const JSONReader& reader, int category) {
			reader.get_float(category, "rotation", &rotation);
		}
	};

	class PointEmitterModule : public ParticleModule {

	public:
		PointEmitterModule() : ParticleModule() {}
		virtual ~PointEmitterModule() {}
		void generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end);
		void  update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt) {}
		const char* getName() const {
			return "point_location";
		}
		const ParticleModuleType getType() const {
			return PM_POINT;
		}
		int getDataSize() const {
			return sizeof(float);
		}
		void debug(const ParticleModuleData* data, void* buffer, uint32_t count) {

		}
	};

}