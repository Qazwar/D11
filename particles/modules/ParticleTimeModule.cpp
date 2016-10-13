#include "ParticleTimeModule.h"
#include <core\profiler\Profiler.h>

namespace ds {

	// -------------------------------------------------------
	// ParticleTimeModule
	// -------------------------------------------------------
	void ParticleTimeModule::generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end) {
		uint32_t count = end - start;
		const LifetimeModuleData* my_data = static_cast<const LifetimeModuleData*>(data);
		float* timers = static_cast<float*>(buffer);
		for (uint32_t i = 0; i < count; ++i) {
			float ttl = math::random(my_data->ttl - my_data->variance, my_data->ttl + my_data->variance);
			array->timer[start + i] = Vector3f(0.0f, 0.0f, ttl);
			timers[start + i] = ttl;
		}

	}

	void  ParticleTimeModule::update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt) {
		ZoneTracker("PM:lifetime");
		float* timers = static_cast<float*>(buffer);
		for (uint32_t i = 0; i < array->countAlive; ++i) {
			array->timer[i].x += dt;
			array->timer[i].y = array->timer[i].x / *timers;
			++timers;
		}
	}

	void ParticleTimeModule::debug(const ParticleModuleData* data, void* buffer, uint32_t count) {
		float* timers = static_cast<float*>(buffer);
		for (uint32_t i = 0; i < count; ++i) {
			LOG << "timer: (" << i << ") : " << timers[i];
		}
	}

}