#include "WiggleModule.h"
#include <core\profiler\Profiler.h>
#include <core\base\Assert.h>

namespace ds {

	// -------------------------------------------------------
	// Acceleration Module
	// -------------------------------------------------------
	void WiggleModule::generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end) {
		XASSERT(data != 0, "Required data not found");
		uint32_t count = end - start;
		const WiggleModuleData* my_data = static_cast<const WiggleModuleData*>(data);
		v2* wiggles = static_cast<v2*>(buffer);
		for (uint32_t i = start; i < end; ++i) {
			float f = math::randomRange(my_data->frequency, my_data->frequencyVariance);
			float a = math::randomRange(my_data->amplitude, my_data->amplitudeVariance);
			wiggles[i] = v2(f,a);
		}
	}

	void WiggleModule::update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt) {
		ZoneTracker("PM:wiggle");
		XASSERT(data != 0, "Required data not found");
		const WiggleModuleData* my_data = (WiggleModuleData*)data;
		v2* wiggles = (v2*)buffer;
		for (uint32_t i = 0; i < array->countAlive; ++i) {
			float r = array->rotation[i];
			v2 n = v2(cos(r), sin(r));
			v2 f = n * sin(array->timer[i].x * wiggles[i].x) * wiggles[i].y;			
			array->forces[i] += v3(f);
		}
	}

	void WiggleModule::debug(const ParticleModuleData* data, void* buffer, uint32_t count) {
		v2* wiggles = static_cast<v2*>(buffer);
		for (uint32_t i = 0; i < count; ++i) {
			LOG << "wiggle: (" << i << ") : frequency: " << wiggles[i].x << " amplitude: " << wiggles[i].y;
		}
	}

}