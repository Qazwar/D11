#include "RingEmitterModule.h"
#include <core\profiler\Profiler.h>
#include <core\base\Assert.h>

namespace ds {

	// -------------------------------------------------------
	// Ring Location Module
	// -------------------------------------------------------
	void RingEmitterModule::generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end) {
		ZoneTracker z("RingLocationModule:generate");
		uint32_t count = end - start;
		const RingEmitterModuleData* my_data = static_cast<const RingEmitterModuleData*>(data);
		float angleVariance = DEGTORAD(my_data->angleVariance);
		float step = TWO_PI / static_cast<float>(count);
		if (my_data->step != 0.0f) {
			step = DEGTORAD(my_data->step);
		}
		for (uint32_t i = 0; i < count; ++i) {
			float myAngle = m_Angle + math::random(-angleVariance, angleVariance);
			float rad = math::random(my_data->radius - my_data->variance, my_data->radius + my_data->variance);
			array->position[start + i].x = array->position[start + i].x + rad * math::fastCos(myAngle);
			array->position[start + i].y = array->position[start + i].y + rad * math::fastSin(myAngle);
			array->position[start + i].z = array->position[start + i].z;
			array->rotation[start + i] = myAngle;
			m_Angle += step;
		}
	}


}