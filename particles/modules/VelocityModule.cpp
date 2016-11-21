#include "VelocityModule.h"
#include <core\profiler\Profiler.h>
#include <core\base\Assert.h>

namespace ds {

	// -------------------------------------------------------
	// VelocityModule
	// -------------------------------------------------------
	void VelocityModule::generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end) {
		XASSERT(data != 0, "Required data not found");
		uint32_t count = end - start;
		const VelocityModuleData* my_data = static_cast<const VelocityModuleData*>(data);
		v2* velocities = static_cast<v2*>(buffer);
		if (my_data->type == VelocityModuleData::VT_RADIAL) {
			for (uint32_t i = 0; i < count; ++i) {
				float v = math::randomRange(my_data->radial, my_data->radialVariance);
				velocities[start + i] = math::getRadialVelocity(array->rotation[start + i], v);
			}
		}
		else {
			for (uint32_t i = 0; i < count; ++i) {
				v2 v = math::randomRange(my_data->velocity, my_data->variance);
				velocities[start + i] = v;
			}
		}
	}

	void VelocityModule::update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt) {
		ZoneTracker("PM:velocity");
		XASSERT(data != 0, "Required data not found");
		const VelocityModuleData* my_data = (VelocityModuleData*)data;
		v2* velocities = (v2*)buffer;
		if (my_data->useDistribution) {
			v2 dist;
			for (uint32_t i = 0; i < array->countAlive; ++i) {
				my_data->distribution.get(array->timer[i].x / array->timer[i].z, &dist);
				float vx = velocities[i].x * dist.x;
				float vy = velocities[i].y * dist.y;
				array->forces[i] += v3(vx, vy, 0.0f);
			}
		}
		else {
			for (uint32_t i = 0; i < array->countAlive; ++i) {
				array->forces[i] += v3(velocities[i], 0.0f);
			}
		}
	}

	void VelocityModule::debug(const ParticleModuleData* data, void* buffer, uint32_t count) {
		v2* velocities = static_cast<v2*>(buffer);
		for (uint32_t i = 0; i < count; ++i) {
			LOG << "velocity: (" << i << ") : " << velocities[i];
		}
	}
}