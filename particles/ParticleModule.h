#pragma once
#include "Particle.h"
#include "..\io\json.h"

namespace ds {

	// -------------------------------------------------------
	// ParticleModuleType
	// -------------------------------------------------------
	enum ParticleModuleType {
		PM_RING,
		PM_LIFECYCLE,
		PM_LINEAR_COLOR,
		PM_SIZE,
		PM_COLOR,
		PM_ALPHA,
		PM_ROTATION,
		PM_VELOCITY,
		PM_ACCELERATION
	};

	enum ModuleModifierType {
		MMT_NONE,
		MMT_PATH,
		MMT_LINEAR,
		MMT_EOL
	};

	// -------------------------------------------------------
	// ParticleModuleData
	// -------------------------------------------------------
	struct ParticleModuleData {

		virtual void read(const JSONReader& reader, int category) = 0;
		virtual void save(JSONWriter& writer) = 0;

	};

	// -------------------------------------------------------
	// ParticleModule
	// -------------------------------------------------------
	class ParticleModule {

	public:
		ParticleModule() {}
		virtual ~ParticleModule() {}
		//virtual void generate(ParticleArray* array, const ParticleModuleData* data, float dt, uint32_t start, uint32_t end) = 0;
		//virtual void update(ParticleArray* array, const ParticleModuleData* data, float dt) = 0;
		virtual void generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end) = 0;
		virtual void update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt) = 0;
		virtual const ParticleModuleType getType() const = 0;
		virtual const char* getName() const = 0;
		virtual int getDataSize() const = 0;
	};

	// -------------------------------------------------------
	// LifetimeModuleData
	// -------------------------------------------------------
	struct LifetimeModuleData : ParticleModuleData {

		float ttl;
		float variance;

		LifetimeModuleData() : ParticleModuleData() , ttl(1.0f), variance(0.0f) {}

		void save(JSONWriter& writer) {
			writer.write("ttl", ttl);
			writer.write("variance", variance);
		}

		void read(const JSONReader& reader, int category) {
			reader.get_float(category, "ttl", &ttl);
			reader.get_float(category, "variance", &variance);
		}

		
	};

	// -------------------------------------------------------
	// ParticleTimeModule
	// -------------------------------------------------------
	class ParticleTimeModule : public ParticleModule {

	public:
		ParticleTimeModule() : ParticleModule() {}
		virtual ~ParticleTimeModule() {}
		void generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end);
		void  update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt);
		const ParticleModuleType getType() const {
			return PM_LIFECYCLE;
		}
		const char* getName() const {
			return "lifecycle";
		}

		int getDataSize() const {
			return sizeof(float);
		}
	};

	// -------------------------------------------------------
	// Ring Location Module
	// -------------------------------------------------------
	struct RingLocationModuleData : ParticleModuleData {

		float radius;
		float variance;
		float angleVariance;
		float step;

		RingLocationModuleData() : ParticleModuleData() , radius(10.0f), variance(0.0f), angleVariance(0.0f), step(0.0f) {}

		void save(JSONWriter& writer) {
			writer.write("radius", radius);
			writer.write("variance", variance);
			writer.write("angle_variance", angleVariance);
			writer.write("step", step);
		}

		void read(const JSONReader& reader, int category) {
			reader.get_float(category, "radius", &radius);
			reader.get_float(category, "variance", &variance);
			reader.get_float(category, "angle_variance", &angleVariance);
			reader.get_float(category, "step", &step);
		}
	};

	class RingLocationModule : public ParticleModule {

	public:
		RingLocationModule() : ParticleModule() {
			m_Angle = 0.0f;
		}
		virtual ~RingLocationModule() {}
		void generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end);
		void  update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt) {}
		const char* getName() const {
			return "ring_location";
		}
		const ParticleModuleType getType() const {
			return PM_RING;
		}
		int getDataSize() const {
			return sizeof(float);
		}
	private:
		float m_Angle;
	};

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

		SizeModuleData() : initial(1,1) , variance(0,0), minScale(0, 0), maxScale(1, 1) , modifier(MMT_NONE) {}

		void save(JSONWriter& writer) {
			writer.write("initial", initial);
			writer.write("variance", variance);			
			if (modifier == MMT_PATH) {
				writer.write("path", path);
			}
			else if (modifier == MMT_LINEAR) {
				writer.write("min", minScale);
				writer.write("max", maxScale);
			}
		}

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
			return sizeof(v2) * 2;
		}
	};

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
			valueVariance(0.0f), alpha(255.0f) , startColor(255,255,255,255) , endColor(0,0,0,0) , modifier(MMT_NONE) {}

		void save(JSONWriter& writer) {
			if (useColor) {
				writer.write("color", color);
			}
			else {
				writer.write("hsv", hsv);
				writer.write("alpha", alpha);
				writer.write("hue_variance", hueVariance);
				writer.write("saturation_variance", saturationVariance);
				writer.write("value_variance", valueVariance);
				
			}
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
	};

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

		AlphaModuleData() : initial(1.0f) , variance(0.0f) , startAlpha(1.0f), endAlpha(0.0f) , modifier(MMT_NONE) {}

		void save(JSONWriter& writer) {
			writer.write("initial", initial);
			writer.write("variance", variance);
			if (modifier == MMT_LINEAR) {
				writer.write("min", startAlpha);
				writer.write("max", endAlpha);
			}
			else if (modifier == MMT_PATH) {
				writer.write("path", path);
			}
		}

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
			return sizeof(float);
		}
	};

	// -------------------------------------------------------
	// Rotation Velocity Module
	// -------------------------------------------------------
	struct RotationModuleData : ParticleModuleData {

		float velocity;
		float variance;

		RotationModuleData() : velocity(0.0f), variance(0.0f) {}

		void save(JSONWriter& writer) {
			writer.write("velocity", RADTODEG(velocity));
			writer.write("variance", RADTODEG(variance));
		}

		void read(const JSONReader& reader, int category) {
			reader.get_float(category, "velocity", &velocity);
			reader.get_float(category, "variance", &variance);
			velocity = DEGTORAD(velocity);
			variance = DEGTORAD(variance);
		}
	};

	class RotationModule : public ParticleModule {

	public:
		RotationModule() : ParticleModule() {}
		virtual ~RotationModule() {}
		void generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end);
		void  update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt);
		const char* getName() const {
			return "rotation";
		}
		const ParticleModuleType getType() const {
			return PM_ROTATION;
		}
		int getDataSize() const {
			return sizeof(float);
		}
	};

	// -------------------------------------------------------
	// Acceleration Module
	// -------------------------------------------------------
	struct AccelerationModuleData : ParticleModuleData {

		float radial;
		float radialVariance;
		v2 acceleration;

		AccelerationModuleData() : radial(0.0f), radialVariance(0.0f) , acceleration(0,0) {}

		void save(JSONWriter& writer) {
			writer.write("radial", radial);
			writer.write("radial_variance", radialVariance);
		}

		void read(const JSONReader& reader, int category) {
			reader.get_float(category, "radial", &radial);
			reader.get_float(category, "radial_variance", &radialVariance);
		}
	};

	class AccelerationModule : public ParticleModule {

	public:
		AccelerationModule() : ParticleModule() {}
		virtual ~AccelerationModule() {}
		void generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end);
		void  update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt);
		const char* getName() const {
			return "acceleration";
		}
		const ParticleModuleType getType() const {
			return PM_ACCELERATION;
		}
		int getDataSize() const {
			return sizeof(v2) * 2;
		}
	};

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

		void save(JSONWriter& writer) {
			writer.write("velocity", velocity);
			writer.write("variance", variance);
		}

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
	};

}