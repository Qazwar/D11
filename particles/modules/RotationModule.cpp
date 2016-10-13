#include "RotationModule.h"
#include <core\profiler\Profiler.h>
#include <core\base\Assert.h>

namespace ds {

	// -------------------------------------------------------
	// Rotation Velocity Module
	// -------------------------------------------------------
	void RotationModule::generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end) {
		XASSERT(data != 0, "Required data not found");
		uint32_t count = end - start;
		float* rotations = (float*)buffer;
		const RotationModuleData* my_data = (RotationModuleData*)data;
		for (uint32_t i = start; i < end; ++i) {
			rotations[i] = math::random(my_data->velocityRange.x, my_data->velocityRange.y);
		}
	}

	void RotationModule::update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt) {
		ZoneTracker("PM:rotation");
		XASSERT(data != 0, "Required data not found");
		const RotationModuleData* my_data = (RotationModuleData*)data;
		float* rotations = (float*)buffer;
		for (uint32_t i = 0; i < array->countAlive; ++i) {
			array->rotation[i] += rotations[i] * dt;
		}
	}

}
