#include "ParticleSystemFactory.h"
#include "ParticleSystem.h"
#include "modules\ParticleTimeModule.h"
#include "modules\SizeModule.h"
#include "modules\ColorModule.h"
#include "modules\RotationModule.h"
#include "modules\VelocityModule.h"
#include "modules\AlphaModule.h"
#include "modules\AccelerationModule.h"
#include "modules\RingEmitterModule.h"
#include "modules\PointEmitterModule.h"
#include "modules\WiggleModule.h"

namespace ds {

	ParticleSystemFactory::ParticleSystemFactory() {
		_count_modules = 0;
		_known_modules = new ParticleModule*[64];
		_known_modules[_count_modules++] = new RingEmitterModule();
		_known_modules[_count_modules++] = new ParticleTimeModule();
		_known_modules[_count_modules++] = new SizeModule();
		_known_modules[_count_modules++] = new ColorModule();
		_known_modules[_count_modules++] = new AlphaModule();
		_known_modules[_count_modules++] = new RotationModule();
		_known_modules[_count_modules++] = new VelocityModule();
		_known_modules[_count_modules++] = new AccelerationModule();
		_known_modules[_count_modules++] = new WiggleModule();
		_known_modules[_count_modules++] = new PointEmitterModule();
		for (int i = 0; i < _count_modules; ++i) {
			_known_module_names.push_back(gStringBuffer->append(_known_modules[i]->getName()));
		}
	}


	ParticleSystemFactory::~ParticleSystemFactory()	{
		for (int i = 0; i < _count_modules; ++i) {
			delete _known_modules[i];
		}
		delete[] _known_modules;
	}
	
	ParticleModuleData* ParticleSystemFactory::addModule(ParticleSystem* system, const char* moduleName) {
		for (int i = 0; i < _count_modules; ++i) {
			if (strcmp(_known_modules[i]->getName(),moduleName) == 0) {
				ParticleModuleData* data = createData(_known_modules[i]->getType());
				system->addModule(_known_modules[i], data);
				return data;
			}
		}
		return 0;
	}

	ParticleModule* ParticleSystemFactory::getModule(const char* moduleName) {
		for (int i = 0; i < _count_modules; ++i) {
			if (strcmp(_known_modules[i]->getName(), moduleName) == 0) {
				return _known_modules[i];
			}
		}
		return 0;
	}
	
	ParticleModuleData* ParticleSystemFactory::createData(ParticleModuleType type) const {
		switch (type) {
			case PM_LIFECYCLE: return new LifetimeModuleData(); break;
			case PM_RING: return new RingEmitterModuleData(); break;
			case PM_SIZE: return new SizeModuleData(); break;
			case PM_COLOR: return new ColorModuleData(); break;
			case PM_ALPHA: return new AlphaModuleData(); break;
			case PM_ROTATION: return new RotationModuleData(); break;
			case PM_VELOCITY: return new VelocityModuleData(); break;
			case PM_ACCELERATION: return new AccelerationModuleData(); break;
			case PM_WIGGLE: return new WiggleModuleData(); break;
			case PM_POINT: return new PointEmitterModuleData(); break;
		}
		return 0;
	}

	
}
