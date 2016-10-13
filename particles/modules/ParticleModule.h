#pragma once
#include "..\Particle.h"
#include "core\io\json.h"

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

	};

	// -------------------------------------------------------
	// ParticleModule
	// -------------------------------------------------------
	class ParticleModule {

	public:
		ParticleModule() {}
		virtual ~ParticleModule() {}

		virtual void generate(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt, uint32_t start, uint32_t end) = 0;

		virtual void update(ParticleArray* array, const ParticleModuleData* data, void* buffer, float dt) = 0;

		virtual const ParticleModuleType getType() const = 0;

		virtual const char* getName() const = 0;

		virtual int getDataSize() const = 0;

		virtual void debug(const ParticleModuleData* data, void* buffer,uint32_t count) = 0;
	};

}