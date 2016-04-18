#pragma once
#include "..\lib\collection_types.h"
#include "ParticleEmitter.h"
#include "ParticleModule.h"

namespace ds {

	class ParticleSystem;

	class ParticleSystemFactory {

	public:
		ParticleSystemFactory();
		~ParticleSystemFactory();
		ParticleModuleData* addModule(ParticleSystem* system, const char* moduleName);
	private:
		ParticleModuleData* createData(ParticleModuleType type) const;
		ParticleSystemFactory(const ParticleSystemFactory& other) {}
		Array<int> _known_module_names;
		ParticleModule** _known_modules;
		int _count_modules;
	};

}

