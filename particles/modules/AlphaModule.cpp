#include "AlphaModule.h"
#include <core\profiler\Profiler.h>
#include <core\base\Assert.h>

namespace ds {

	// -------------------------------------------------------
	// Alpha Module
	// -------------------------------------------------------
	void AlphaModule::update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt) {
		ZoneTracker("PM:alpha");
		XASSERT(data != 0, "Required data not found");
		const AlphaModuleData* my_data = (AlphaModuleData*)data;
		v2* alphas = (v2*)buffer;
		if (my_data->modifier != MMT_NONE) {
			if (my_data->modifier == MMT_LINEAR) {
				for (uint32_t i = 0; i < array->countAlive; ++i) {
					array->color[i].a = alphas[i].x * (1.0f - array->timer[i].y) + alphas[i].y * array->timer[i].y;
				}
			}
			else {
				float a = 0.0f;
				for (uint32_t i = 0; i < array->countAlive; ++i) {
					my_data->path.get(array->timer[i].y, &a);
					array->color[i].a = math::clamp(a, 0.0f, 1.0f);
				}
			}
		}
	}

	void AlphaModule::generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end) {
		XASSERT(data != 0, "Required data not found");
		const AlphaModuleData* my_data = static_cast<const AlphaModuleData*>(data);
		v2* alphas = (v2*)buffer;
		for (uint32_t i = start; i < end; ++i) {
			float start = math::clamp(math::randomRange(my_data->initial, my_data->variance), 0.0f, 1.0f);
			alphas[i] = v2(start, my_data->endAlpha);
			array->color[i].a = start;
		}
	}

	void AlphaModule::debug(const ParticleModuleData* data, void* buffer, uint32_t count) {

	}

}