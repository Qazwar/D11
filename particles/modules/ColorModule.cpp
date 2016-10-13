#include "ColorModule.h"
#include <core\profiler\Profiler.h>
#include <core\base\Assert.h>

namespace ds {

	// -------------------------------------------------------
	// Color Module
	// -------------------------------------------------------
	void ColorModule::generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end) {
		XASSERT(data != 0, "Required data not found");
		uint32_t count = end - start;
		const ColorModuleData* my_data = (ColorModuleData*)data;
		if (my_data->modifier == MMT_LINEAR) {
			for (uint32_t i = 0; i < count; ++i) {
				array->color[start + i] = my_data->startColor;
			}
		}
		else if (my_data->modifier == MMT_PATH) {
			for (uint32_t i = 0; i < count; ++i) {
				array->color[start + i] = my_data->path.value(0);
			}
		}
		else {
			if (my_data->useColor) {
				for (uint32_t i = 0; i < count; ++i) {
					array->color[start + i] = my_data->color;
				}
			}
			else {
				for (uint32_t i = 0; i < count; ++i) {
					float hv = math::random(-my_data->hueVariance, my_data->hueVariance);
					float h = math::clamp(my_data->hsv.x + hv, 0.0f, 360.0f);
					float sv = math::random(-my_data->saturationVariance, my_data->saturationVariance);
					float s = math::clamp(my_data->hsv.y + sv, 0.0f, 100.0f);
					float vv = math::random(-my_data->valueVariance, my_data->valueVariance);
					float v = math::clamp(my_data->hsv.z + vv, 0.0f, 100.0f);
					Color c = color::hsvToColor(h, s, v);
					if (my_data->alpha > 1.0f) {
						c.a = my_data->alpha / 255.0f;
					}
					else {
						c.a = my_data->alpha;
					}
					array->color[start + i] = c;
				}
			}
		}
	}

	void ColorModule::update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt) {
		ZoneTracker("PM:color");
		XASSERT(data != 0, "Required data not found");
		const ColorModuleData* my_data = (ColorModuleData*)data;
		if (my_data->modifier == MMT_LINEAR) {
			for (uint32_t i = 0; i < array->countAlive; ++i) {
				array->color[i] = color::lerp(my_data->startColor, my_data->endColor, array->timer[i].y);
			}
		}
		else if (my_data->modifier == MMT_PATH) {
			for (uint32_t i = 0; i < array->countAlive; ++i) {
				my_data->path.get(array->timer[i].y, &array->color[i]);
			}
		}
	}

}