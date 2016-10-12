#include "ParticleModule.h"
#include "core\profiler\Profiler.h"
#include "core\base\Assert.h"
#include "core\math\math.h"

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

	// -------------------------------------------------------
	// Ring Location Module
	// -------------------------------------------------------
	void RingLocationModule::generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end) {
		ZoneTracker z("RingLocationModule:generate");
		uint32_t count = end - start;
		const RingLocationModuleData* my_data = static_cast<const RingLocationModuleData*>(data);
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
	
	// -------------------------------------------------------
	// Size Module
	// -------------------------------------------------------
	void SizeModule::update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt) {
		ZoneTracker("PM:size");
		XASSERT(data != 0, "Required data not found");
		const SizeModuleData* my_data = static_cast<const SizeModuleData*>(data);
		v2* scales = static_cast<v2*>(buffer);
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

	// -------------------------------------------------------
	// Color Module
	// -------------------------------------------------------
	void ColorModule::generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end) {
		XASSERT(data != 0, "Required data not found");
		uint32_t count = end - start;
		const ColorModuleData* my_data = static_cast<const ColorModuleData*>(data);
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
		const ColorModuleData* my_data = static_cast<const ColorModuleData*>(data);
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

	// -------------------------------------------------------
	// Alpha Module
	// -------------------------------------------------------
	void AlphaModule::update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt) {
		ZoneTracker("PM:alpha");
		XASSERT(data != 0, "Required data not found");
		const AlphaModuleData* my_data = static_cast<const AlphaModuleData*>(data);
		v2* alphas = static_cast<v2*>(buffer);
		if (my_data->modifier != MMT_NONE) {
			if (my_data->modifier == MMT_LINEAR) {
				for (uint32_t i = 0; i < array->countAlive; ++i) {
					array->color[i].a = alphas[i].x * ( 1.0f - array->timer[i].y) + alphas[i].y * array->timer[i].y;
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
		v2* alphas = static_cast<v2*>(buffer);
		for (uint32_t i = start; i < end; ++i) {
			float start = math::clamp(math::randomRange(my_data->initial, my_data->variance), 0.0f, 1.0f);
			alphas[i] = v2(start, my_data->endAlpha);
			array->color[i].a = start;
		}
	}

	void AlphaModule::debug(const ParticleModuleData* data, void* buffer, uint32_t count) {

	}

	// -------------------------------------------------------
	// Rotation Velocity Module
	// -------------------------------------------------------
	void RotationModule::generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end) {
		XASSERT(data != 0, "Required data not found");
		uint32_t count = end - start;
		float* rotations = static_cast<float*>(buffer);
		const RotationModuleData* my_data = static_cast<const RotationModuleData*>(data);
		for (uint32_t i = start; i < end; ++i) {
			rotations[i] = math::random(my_data->velocityRange.x, my_data->velocityRange.y);
		}
	}

	void RotationModule::update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt) {
		ZoneTracker("PM:rotation");
		XASSERT(data != 0, "Required data not found");
		const RotationModuleData* my_data = static_cast<const RotationModuleData*>(data);
		float* rotations = static_cast<float*>(buffer);
		for (uint32_t i = 0; i < array->countAlive; ++i) {
			array->rotation[i] += rotations[i] * dt;
		}
	}

	// -------------------------------------------------------
	// Acceleration Module
	// -------------------------------------------------------
	void AccelerationModule::generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end) {
		XASSERT(data != 0, "Required data not found");
		uint32_t count = end - start;
		const AccelerationModuleData* my_data = static_cast<const AccelerationModuleData*>(data);
		v2* accelerations = static_cast<v2*>(buffer);
		accelerations += start * 2;
		for (uint32_t i = 0; i < count; ++i) {
			float v = math::randomRange(my_data->radial, my_data->radialVariance);
			*accelerations = math::getRadialVelocity(array->rotation[start + i], v);
			++accelerations;
			*accelerations = v2(0, 0);
			++accelerations;
		}
	}

	void AccelerationModule::update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt) {
		ZoneTracker("PM:acceleration");
		XASSERT(data != 0, "Required data not found");
		const AccelerationModuleData* my_data = static_cast<const AccelerationModuleData*>(data);
		v2* accelerations = static_cast<v2*>(buffer);
		for (uint32_t i = 0; i < array->countAlive; ++i) {
			accelerations[i * 2 + 1] += accelerations[i * 2] * dt;
			array->forces[i] += v3(accelerations[i * 2 + 1]);
		}
	}

	void AccelerationModule::debug(const ParticleModuleData* data, void* buffer, uint32_t count) {
		v2* accelerations = static_cast<v2*>(buffer);
		for (uint32_t i = 0; i < count; ++i) {
			LOG << "acceleration: (" << i << ") : " << DBG_V2(accelerations[i * 2]) << " velocity: " << DBG_V2(accelerations[i * 2 + 1]);
		}
	}

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
		const VelocityModuleData* my_data = static_cast<const VelocityModuleData*>(data);
		v2* velocities = static_cast<v2*>(buffer);
		if (my_data->useDistribution) {
			v2 dist;
			for (uint32_t i = 0; i < array->countAlive; ++i) {
				my_data->distribution.get(array->timer[i].x / array->timer[i].z, &dist);
				float vx = velocities[i].x * dist.x;
				float vy = velocities[i].y * dist.y;
				array->forces[i] += v3(vx,vy,0.0f);
			}
		}
		else {
			for (uint32_t i = 0; i < array->countAlive; ++i) {
				array->forces[i] += v3(velocities[i],0.0f);
			}
		}
	}

	void VelocityModule::debug(const ParticleModuleData* data, void* buffer, uint32_t count) {
		v2* velocities = static_cast<v2*>(buffer);
		for (uint32_t i = 0; i < count; ++i) {
			LOG << "velocity: (" << i << ") : " << DBG_V2(velocities[i]);
		}
	}
}