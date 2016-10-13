#include "SizeModule.h"
#include <core\profiler\Profiler.h>
#include <core\base\Assert.h>

namespace ds {

	// -------------------------------------------------------
	// Size Module
	// -------------------------------------------------------
	void SizeModule::update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt) {
		ZoneTracker("PM:size");
		XASSERT(data != 0, "Required data not found");
		//const SizeModuleData* my_data = static_cast<const SizeModuleData*>(data);
		const SizeModuleData* my_data = (SizeModuleData*)data;
		//v2* scales = static_cast<v2*>(buffer);
		v2* scales = (v2*)buffer;
		if (my_data->modifier != MMT_NONE) {
			for (uint32_t i = 0; i < array->countAlive; ++i) {
				if (my_data->modifier == MMT_PATH) {
					for (uint32_t i = 0; i < array->countAlive; ++i) {
						my_data->path.get(array->timer[i].y, &array->scale[i]);
						array->scale[i].x *= scales[i].x;
						array->scale[i].y *= scales[i].y;
					}
				}
				else {
					array->scale[i] = lerp(my_data->minScale, my_data->maxScale, array->timer[i].y);
					array->scale[i].x *= scales[i].x;
					array->scale[i].y *= scales[i].y;
				}
			}
		}
	}

	void SizeModule::generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end) {
		XASSERT(data != 0, "Required data not found");
		const SizeModuleData* my_data = static_cast<const SizeModuleData*>(data);
		v2* scales = static_cast<v2*>(buffer);
		for (uint32_t i = start; i < end; ++i) {
			v2 s = math::randomRange(my_data->initial, my_data->variance);
			if (s.x < 0.1f) {
				s.x = 0.1f;
			}
			if (s.y < 0.1f) {
				s.y = 0.1f;
			}
			array->scale[i] = s;
			scales[i] = s;
			if (my_data->modifier == MMT_LINEAR) {
				array->scale[i].x = s.x * my_data->minScale.x;
				array->scale[i].y = s.y * my_data->minScale.y;
			}
		}
	}

	void SizeModule::debug(const ParticleModuleData* data, void* buffer, uint32_t count) {
		v2* scales = static_cast<v2*>(buffer);
		for (uint32_t i = 0; i < count; ++i) {
			LOG << "scale: (" << i << ") : " << DBG_V2(scales[i]);
		}
	}
}