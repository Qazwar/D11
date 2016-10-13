#pragma once
#include "ParticleModule.h"

namespace ds {
	// -------------------------------------------------------
	// Size Module
	// -------------------------------------------------------	
	struct SizeModuleData : ParticleModuleData {

		v2 initial;
		v2 variance;
		v2 minScale;
		v2 maxScale;
		Vector2fPath path;
		ModuleModifierType modifier;

		SizeModuleData() : initial(1, 1), variance(0, 0), minScale(0, 0), maxScale(1, 1), modifier(MMT_NONE) {}

		void read(const JSONReader& reader, int category) {
			modifier = MMT_NONE;
			reader.get_vec2(category, "initial", &initial);
			reader.get_vec2(category, "variance", &variance);
			if (reader.contains_property(category, "path")) {
				reader.get_vec2_path(category, "path", &path);
				modifier = MMT_PATH;
			}
			else if (reader.contains_property(category, "min")) {
				reader.get_vec2(category, "min", &minScale);
				reader.get_vec2(category, "max", &maxScale);
				modifier = MMT_LINEAR;
			}
		}
	};

	class SizeModule : public ParticleModule {

	public:
		SizeModule() : ParticleModule() {}
		virtual ~SizeModule() {}
		void update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt);
		void generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end);
		const ParticleModuleType getType() const {
			return PM_SIZE;
		}
		const char* getName() const {
			return "size";
		}
		int getDataSize() const {
			return sizeof(v2);
		}
		void debug(const ParticleModuleData* data, void* buffer, uint32_t count);
	};

}