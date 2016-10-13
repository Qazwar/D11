#pragma once 
#include "ParticleModule.h"

namespace ds {

	// -------------------------------------------------------
	// Velocity Module
	// -------------------------------------------------------
	struct VelocityModuleData : ParticleModuleData {

		enum VelocityType {
			VT_RADIAL,
			VT_NORMAL,
			VT_NONE,
			VT_EOL
		};

		v2 velocity;
		v2 variance;
		float radial;
		float radialVariance;
		Vector2fPath distribution;
		VelocityType type;
		bool useDistribution;

		VelocityModuleData() : velocity(0, 0), variance(0, 0), radial(0.0f), radialVariance(0), type(VT_NONE), useDistribution(false) {}

		void read(const JSONReader& reader, int category) {
			if (reader.contains_property(category, "velocity")) {
				reader.get_vec2(category, "velocity", &velocity);
				reader.get_vec2(category, "variance", &variance);
				type = VT_NORMAL;
			}
			if (reader.contains_property(category, "radial")) {
				reader.get_float(category, "radial", &radial);
				reader.get_float(category, "radial_variance", &radialVariance);
				type = VT_RADIAL;
			}
			if (reader.contains_property(category, "distribution")) {
				useDistribution = true;
				reader.get_vec2_path(category, "distribution", &distribution);
			}
		}
	};

	class VelocityModule : public ParticleModule {

	public:
		VelocityModule() : ParticleModule() {}
		virtual ~VelocityModule() {}
		void generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end);
		void update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt);
		const char* getName() const {
			return "velocity";
		}
		const ParticleModuleType getType() const {
			return PM_VELOCITY;
		}
		int getDataSize() const {
			return sizeof(v2);
		}
		void debug(const ParticleModuleData* data, void* buffer, uint32_t count);
	};

}