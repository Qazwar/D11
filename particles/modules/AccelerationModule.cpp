#include "AccelerationModule.h"
#include <core\profiler\Profiler.h>
#include <core\base\Assert.h>

namespace ds {

	// -------------------------------------------------------
	// Acceleration Module
	// -------------------------------------------------------
	void AccelerationModule::generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end) {
		XASSERT(data != 0, "Required data not found");
		uint32_t count = end - start;
		const AccelerationModuleData* my_data = static_cast<const AccelerationModuleData*>(data);
		v2* accelerations = static_cast<v2*>(buffer);
		accelerations += start * 3;
		for (uint32_t i = 0; i < count; ++i) {
			float v = math::randomRange(my_data->radial, my_data->radialVariance);
			*accelerations = math::getRadialVelocity(array->rotation[start + i], v); // acceleration
			++accelerations;
			*accelerations = v2(0,0); // velocity
			++accelerations;
			*accelerations = v2(my_data->damping, 0.0f); // damping
			++accelerations;
		}
	}

	void AccelerationModule::update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt) {
		ZoneTracker("PM:acceleration");
		XASSERT(data != 0, "Required data not found");
		const AccelerationModuleData* my_data = (AccelerationModuleData*)data;
		v2* accelerations = (v2*)buffer;
		for (uint32_t i = 0; i < array->countAlive; ++i) {
			accelerations[i * 3 + 1] += accelerations[i * 3] * dt;
			accelerations[i * 3] *= 1.0f - accelerations[i * 3 + 2].x;// *dt;
			array->forces[i] += v3(accelerations[i * 3 + 1]);
		}
	}

	void AccelerationModule::debug(const ParticleModuleData* data, void* buffer, uint32_t count) {
		v2* accelerations = static_cast<v2*>(buffer);
		for (uint32_t i = 0; i < count; ++i) {
			LOG << "acceleration: (" << i << ") : " << DBG_V2(accelerations[i * 2]) << " velocity: " << DBG_V2(accelerations[i * 2 + 1]);
		}
	}

}