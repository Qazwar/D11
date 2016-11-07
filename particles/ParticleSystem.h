#pragma once
#include "core\io\DataFile.h"
#include "Particle.h"
#include "core\profiler\Profiler.h"
#include "ParticleSystemFactory.h"
#include "core\lib\collection_types.h"
#include "modules\ParticleModule.h"

namespace ds {

	struct ParticleEvent {
		enum ParticleEventType {
			SYSTEM_STARTED,
			SYSTEM_KILLED,
			PARTICLE_EMITTED,
			PARTICLE_KILLED,
			EOL
		};
		ID instance;
		v2 pos;
		ParticleEventType type;
	};

	enum ParticleRenderMode {
		PRM_2D,
		PRM_3D
	};

	// -------------------------------------------------------
	// Module instance
	// -------------------------------------------------------
	struct ModuleInstance {

		ParticleModule* module;
		ParticleModuleData* data;

		ModuleInstance() : module(0), data(0) {}
	};

// -------------------------------------------------------
// Particle system
// -------------------------------------------------------
class ParticleSystem : public JSONAssetFile {

public:
	ParticleSystem(int id, const char* name, const char* fileName, ParticleSystemFactory* factory, ParticleRenderMode renderMode);
	~ParticleSystem();
	void clear();
	void update(float elapsed, Array<ParticleEvent>& events);
	ID start(const v2& startPosition);
	void stop(ID id);
	void addModule(ParticleModule* module, ParticleModuleData* data) {
		if (_count_modules < 32) {
			ModuleInstance& instance = _module_instances[_count_modules];
			_sizes[_count_modules] = module->getDataSize();
			instance.module = module;
			instance.data = data;
			++_count_modules;
		}
	}
	const ModuleInstance& getModuleInstance(int id) const {
		return _module_instances[id];
	}
	void getModuleNames(Array<const char*>& names);
	ParticleModuleData* getData(const char* modifierName);
	ParticleModuleData* getData(ParticleModuleType type);
	ParticleSpawner* getSpawner() {
		return &_spawner;
	}
	const int getCountAlive() const {
		return m_Array.countAlive;
	}
	const char* getDebugName() const {
		return m_DebugName;
	}
	
	int getID() const {
		return _id;
	}

	virtual bool loadData(const JSONReader& loader);
	virtual bool reloadData(const JSONReader& loader) {
		return loadData(loader);
	}
	bool isAlive() const {
		return _spawnerInstances.numObjects > 0 || m_Array.countAlive > 0;
	}
	void activateEvents() {
		_sendEvents = true;
	}
	void debug();
	const ParticleRenderMode getRenderMode() const {
		return _renderMode;
	}
	const ParticleArray& getArray() const {
		return m_Array;
	}
	const Texture& getTexture() const {
		return _texture;
	}
private:
	void updateSpawners(float dt);
	void initSpawner();
	void emittParticles(ParticleSpawnerInstance& instance, float dt, uint32_t* start, uint32_t* end);
	void emittParticles(const ParticleSpawnerInstance& instance, int count, uint32_t* start, uint32_t* end, float dt);
	void prepareVertices();
	ParticleSpawner _spawner;
	Texture _texture;
	ParticleArray m_Array;
	int _sizes[32];
	BlockArray _buffer;
	char m_DebugName[32];
	char _json_name[64];
	int _id;
	ModuleInstance _module_instances[32];
	int _count_modules;
	ParticleSystemFactory* _factory;
	SpawnerInstances _spawnerInstances;
	bool _sendEvents;
	uint32_t _counter;
	ParticleRenderMode _renderMode;

	float _dbgValues[64];
	int _dbgCounter;
};

}