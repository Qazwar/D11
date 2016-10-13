#pragma once
#include "ParticleModule.h"

namespace ds {

	// -------------------------------------------------------
	// Alpha Module
	// -------------------------------------------------------
	struct AlphaModuleData : ParticleModuleData {

		float initial;
		float variance;
		float startAlpha;
		float endAlpha;
		ModuleModifierType modifier;
		FloatPath path;

		AlphaModuleData() : initial(1.0f), variance(0.0f), startAlpha(1.0f), endAlpha(0.0f), modifier(MMT_NONE) {}

		void read(const JSONReader& reader, int category) {
			modifier = MMT_NONE;
			reader.get_float(category, "initial", &initial);
			reader.get_float(category, "variance", &variance);
			if (reader.contains_property(category, "path")) {
				modifier = MMT_PATH;
				reader.get_float_path(category, "path", &path);
			}
			else if (reader.contains_property(category, "min")) {
				modifier = MMT_LINEAR;
				reader.get_float(category, "min", &startAlpha);
				reader.get_float(category, "max", &endAlpha);
			}
		}
	};

	class AlphaModule : public ParticleModule {

	public:
		AlphaModule() : ParticleModule() {}
		virtual ~AlphaModule() {}
		void  update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt);
		void generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end);
		const ParticleModuleType getType() const {
			return PM_ALPHA;
		}
		const char* getName() const {
			return "alpha";
		}
		int getDataSize() const {
			return sizeof(v2);
		}
		void debug(const ParticleModuleData* data, void* buffer, uint32_t count);
	};

}