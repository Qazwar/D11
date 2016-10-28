#include "PointEmitterModule.h"
#include <core\profiler\Profiler.h>
#include <core\base\Assert.h>

namespace ds {

	// -------------------------------------------------------
	// Ring Location Module
	// -------------------------------------------------------
	void PointEmitterModule::generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end) {
		ZoneTracker z("PointEmitterModule:generate");
		uint32_t count = end - start;
		const PointEmitterModuleData* my_data = static_cast<const PointEmitterModuleData*>(data);
		for (uint32_t i = 0; i < count; ++i) {
			array->position[start + i].x = array->position[start + i].x;
			array->position[start + i].y = array->position[start + i].y;
			array->position[start + i].z = array->position[start + i].z;
			array->rotation[start + i] = my_data->rotation;
		}
	}


}