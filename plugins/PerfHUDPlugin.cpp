#include "PerfHUDPlugin.h"
#include "..\imgui\IMGUI.h"
#include <core\profiler\Profiler.h>
#include "..\renderer\graphics.h"

namespace ds {

	void PerfHUDPlugin::showDialog() {
		v2 p(10, graphics::getScreenHeight() - 10.0f);
		gui::start(1, &p);
		int state = 1;
		gui::begin("Perf HUD", &state);
		float values[16];
		int num = perf::getTimerValues("Duration", values, 16);
		float min = 100000.0f;
		float max = 0.0f;
		for (int i = 0; i < num; ++i) {
			if (values[i] > max) {
				max = values[i];
			}
			if (values[i] < min) {
				min = values[i];
			}
		}
		float tickSpacing = 0.1f;
		float niceMin = std::floor(min);
		float niceMax = std::ceil(max);
		gui::Histogram(values, num, niceMin, niceMax, tickSpacing);
		gui::end();
	}

}