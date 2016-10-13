#pragma once
#include "ParticleModule.h"

namespace ds {

	// -------------------------------------------------------
	// Color Module
	// -------------------------------------------------------
	struct ColorModuleData : ParticleModuleData {

		Color color;
		bool useColor;
		v3 hsv;
		float hueVariance;
		float saturationVariance;
		float valueVariance;
		float alpha;
		Color startColor;
		Color endColor;
		ColorPath path;
		ModuleModifierType modifier;

		ColorModuleData() : color(Color::WHITE), useColor(true), hsv(360, 100, 100), hueVariance(0.0f), saturationVariance(0.0f),
			valueVariance(0.0f), alpha(255.0f), startColor(255, 255, 255, 255), endColor(0, 0, 0, 0), modifier(MMT_NONE) {
		}

		void read(const JSONReader& reader, int category) {
			modifier = MMT_NONE;
			if (reader.contains_property(category, "color")) {
				useColor = true;
				reader.get_color(category, "color", &color);
			}
			else {
				useColor = false;
				reader.get_vec3(category, "hsv", &hsv);
				reader.get_float(category, "alpha", &alpha);
				reader.get_float(category, "hue_variance", &hueVariance);
				reader.get_float(category, "saturation_variance", &saturationVariance);
				reader.get_float(category, "value_variance", &valueVariance);
			}
			if (reader.contains_property(category, "start")) {
				modifier = MMT_LINEAR;
				reader.get_color(category, "start", &startColor);
				reader.get_color(category, "end", &endColor);
			}
			if (reader.contains_property(category, "path")) {
				modifier = MMT_PATH;
				reader.get_color_path(category, "path", &path);
			}
		}

	};

	class  ColorModule : public ParticleModule {

	public:
		ColorModule() : ParticleModule() {
		}
		virtual ~ColorModule() {}
		void generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end);
		void update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt);
		const ParticleModuleType getType() const {
			return PM_COLOR;
		}
		const char* getName() const {
			return "color";
		}
		int getDataSize() const {
			return sizeof(float);
		}
		void debug(const ParticleModuleData* data, void* buffer, uint32_t count) {

		}
	};

}