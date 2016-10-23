#pragma once
#include "ParticleSystem.h"
#include "ParticleSystemFactory.h"
#include "..\resources\ResourceDescriptors.h"
#include "ParticleSystemRenderer.h"

namespace ds {

const int MAX_PARTICLES = 16386;

const int MAX_RENDERER = 2;

struct ParticleSystemGroup {
	uint32_t id;
	char name[20];
	Array<int> systems;
};

struct ParticleEvent;
	
class ParticleManager : public JSONAssetFile {

public:
	ParticleManager(const ParticleSystemsDescriptor& descriptor);
	~ParticleManager();
	//void init(const Descriptor& desc);
	void start(uint32_t id,const v2& pos);
	void startGroup(uint32_t id, const v2& pos);
	void stop(uint32_t id);
	void setBlendState(int blendState) {
		m_BlendState = blendState;
	}
	ParticleSystem* getParticleSystem(uint32_t id) {
		assert(_systems[id] != 0);
		return _systems[id];
	}
	void update(float elapsed);
	void render();

	ParticleSystem* create(int id, const char* name, ParticleRenderMode renderMode);
	ParticleSystem* create(const char* name, ParticleRenderMode renderMode);
	void removeSystem(int id);
	void debug();
	//void fillModel(gui::ComponentModel<int>* model);
	bool saveData(JSONWriter& writer);
	const char* getFileName() const {
		return "particles\\particlesystems.json";
	}
	const ParticleSystemFactory& getFactory() const {
		return _factory;
	}
	virtual bool loadData(const JSONReader& loader);
	bool reloadData(const JSONReader& loader) {
		return loadData(loader);
	}
	bool hasEvents() const {
		return _events.size() > 0;
	}
	int getEventsCount() const {
		return _events.size();
	}
	const ParticleEvent& getEvent(int index) {
		return _events[index];
	}
	int getSystemIds(int* ids, int max);
private:
	int findGroup(uint32_t id);
	ParticleSystem** _systems;
	int m_BlendState;
	//SpriteBuffer* _particles;
	ParticleSystemFactory _factory;
	Array<ParticleSystemGroup> _groups;
	Array<ParticleEvent> _events;
	ParticleSystemRenderer* _renderer[MAX_RENDERER];
};

}
