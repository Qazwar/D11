#include "ParticleManager.h"
#include "core\profiler\Profiler.h"
#include "core\math\matrix.h"
#include "core\log\Log.h"
#include "core\io\FileRepository.h"
#include "..\resources\ResourceContainer.h"

namespace ds {

	const int MAX_PARTICLE_SYSTEMS = 128;

	// --------------------------------------------------------------------------
	// constructor
	// --------------------------------------------------------------------------
	ParticleManager::ParticleManager(const ParticleSystemsDescriptor& descriptor) : JSONAssetFile("content\\particles\\particlesystems.json") {
		_numSystems = 0;
		_systems = new ParticleSystem*[MAX_PARTICLE_SYSTEMS];
		for (int i = 0; i < 128; ++i) {
			_systems[i] = 0;
		}
		_renderer[PRM_2D] = new ParticleSystemRenderer2D(descriptor.spriteBuffer);
	}

	// --------------------------------------------------------------------------
	// destructor
	// --------------------------------------------------------------------------
	ParticleManager::~ParticleManager(void) {
		for (int i = 0; i < MAX_PARTICLE_SYSTEMS; ++i) {
			if (_systems[i] != 0) {
				delete _systems[i];
			}
			_systems[i] = 0;
		}
		delete[] _systems;
		delete _renderer[PRM_2D];
	}

	int ParticleManager::getSystemIds(int* ids, int max) {
		int cnt = 0;
		for (int i = 0; i < MAX_PARTICLE_SYSTEMS; ++i) {
			if (_systems[i] != 0) {
				if (cnt < max) {
					ids[cnt++] = i;
				}
			}
		}
		return cnt;
	}

	ParticleSystem* ParticleManager::create(int id, const char* name, ParticleRenderMode renderMode) {
		char buffer[128];
		sprintf_s(buffer, 128, "content\\particles\\%s.json", name);
		ParticleSystem* system = new ParticleSystem(id, name, buffer, &_factory, renderMode);
		return system;
	}

	ParticleSystem* ParticleManager::create(const char* name, ParticleRenderMode renderMode) {
		int idx = -1;
		for (int i = 0; i < MAX_PARTICLE_SYSTEMS; ++i) {
			if (_systems[i] == 0 && idx == -1) {
				idx = i;
			}
		}
		if (idx != -1) {
			char buffer[128];
			sprintf_s(buffer, 128, "content\\particles\\%s.json", name);
			ParticleSystem* system = new ParticleSystem(idx, name, buffer, &_factory, renderMode);
			_systems[idx] = system;
			return system;
		}
		return 0;
	}

	// --------------------------------------------------------------------------
	// start specific particlesystem
	// --------------------------------------------------------------------------
	void ParticleManager::start(uint32_t id,const v2& pos) {	
		ZoneTracker z("ParticleManager::start");
		ParticleSystem* system = _systems[id];
		assert(system != 0);
		system->start(pos);
	}

	int ParticleManager::findGroup(uint32_t id) {
		for (uint32_t i = 0; i < _groups.size(); ++i) {
			if (_groups[i].id == id) {
				return i;
			}
		}
		return -1;
	}
	// --------------------------------------------------------------------------
	// start specific particlesystem
	// --------------------------------------------------------------------------
	void ParticleManager::startGroup(uint32_t id, const v2& pos) {
		ZoneTracker z("ParticleManager::start");
		int idx = findGroup(id);
		if (idx != -1) {
			const ParticleSystemGroup& group = _groups[idx];
			for (uint32_t i = 0; i < group.systems.size(); ++i) {
				ParticleSystem* system = _systems[group.systems[i]];
				assert(system != 0);
				system->start(pos);
			}
		}
	}

	// --------------------------------------------------------------------------
	// stop specific particlesystem
	// --------------------------------------------------------------------------
	void ParticleManager::stop(uint32_t id) {
		//assert(m_Index[id] != -1);
		//NewParticleSystem* system = m_Systems[m_Index[id]];
		//system->stop();
	}
	/*
	void ParticleManager::fillModel(gui::ComponentModel<int>* model) {
		model->clear();
		char buffer[32];
		for (int i = 0; i < MAX_PARTICLE_SYSTEMS; ++i) {
			if (_systems[i] != 0) {
				sprintf_s(buffer, 32, "%s (%d)", _systems[i]->getDebugName(), _systems[i]->getID());
				LOG << "----> '" << buffer << "'";
				model->add(buffer, _systems[i]->getID());
			}
		}
	}
	*/
	bool ParticleManager::saveData(JSONWriter& writer) {
		for (int i = 0; i < MAX_PARTICLE_SYSTEMS; ++i) {
			if (_systems[i] != 0) {
				writer.startCategory(_systems[i]->getDebugName());
				writer.write("id", i);
				writer.write("file", _systems[i]->getDebugName());
				writer.endCategory();
			}
		}
		if (!_groups.empty()) {
			writer.startCategory("groups");
			for (uint32_t i = 0; i < _groups.size(); ++i) {
				const ParticleSystemGroup& group = _groups[i];
				writer.startCategory(group.name);
				writer.write("id", group.id);
				writer.write("systems", group.systems.data(), group.systems.size());
				writer.endCategory();
			}
			writer.endCategory();
		}
		return true;
	}

	bool ParticleManager::loadData(const JSONReader& reader) {
		LOG << "importing data";
		int cats[256];
		int num = reader.get_categories(cats, 256);
		for (int i = 0; i < num; ++i) {
			LOG << "name: " << reader.get_category_name(cats[i]);
			if (reader.matches(cats[i], "groups")) {
				int groups[256];
				int num_groups = reader.get_categories(groups, 256, cats[i]);
				for (int z = 0; z < num_groups; ++z) {
					const char* name = reader.get_category_name(groups[z]);
					ParticleSystemGroup group;
					sprintf_s(group.name, 20, "%s", name);
					int id = -1;
					reader.get_uint(groups[z], "id", &group.id);
					int sar[32];
					int nr_sar = reader.get_array(groups[z], "systems", sar, 32);
					for (int j = 0; j < nr_sar; ++j) {
						group.systems.push_back(sar[j]);
					}
					_groups.push_back(group);
				}
			}
			else {
				const char* name = reader.get_string(cats[i], "file");
				int id = -1;
				reader.get_int(cats[i], "id", &id);
				bool se = false;
				// FIXME: read render mode : 2D, 3D
				if (reader.contains_property(cats[i], "send_events")) {
					reader.get(cats[i], "send_events", &se);
				}
				if (id != -1) {
					ParticleSystemInfo info;
					strcpy(info.name, name);
					info.id = id;
					ParticleSystem* system = create(id, name, ParticleRenderMode::PRM_2D);
					if (se) {
						system->activateEvents();
					}
					LOG << "id: " << id << " name: " << name;
					system->load();
					repository::add(system);
					//repository::load(system);
					_systems[id] = system;
					_systemInfos.push_back(info);
					++_numSystems;
				}
			}
		}
		return true;
	}

	void  ParticleManager::removeSystem(int id) {
		assert(_systems[id] != 0);
		ParticleSystem* system = _systems[id];
		delete system;
		_systems[id] = 0;
	}
	
	// --------------------------------------------------------------------------
	// update
	// --------------------------------------------------------------------------
	void ParticleManager::update(float elapsed) {
		ZoneTracker z("ParticleManager::update");
		_events.clear();
		for (int i = 0; i < MAX_PARTICLE_SYSTEMS; ++i) {
			if (_systems[i] != 0) {
				if (_systems[i]->isAlive()) {
					_systems[i]->update(elapsed, _events);
				}
			}
		}
	}

	// --------------------------------------------------------------------------
	// render
	// --------------------------------------------------------------------------
	void ParticleManager::render() {
		//_particles->begin();
		int batchSize = 0;
		//_particles->begin();
		ZoneTracker z("ParticleManager::render");
		//SpriteVertex v;
		int total = 0;
		for (int i = 0; i < MAX_PARTICLE_SYSTEMS; ++i) {
			if (_systems[i] != 0) {
				ParticleSystemRenderer* renderer = _renderer[_systems[i]->getRenderMode()];
				const ParticleArray& array = _systems[i]->getArray();
				const Texture& t = _systems[i]->getTexture();
				renderer->render(array, t);
				total += array.countAlive;
				/*
				if (array.countAlive > 0) {
					for (int j = 0; j < array.countAlive; ++j) {
						_particles->draw(array.position[j].xy(), t, array.rotation[j], array.scale[j], array.color[j]);
					}
				}
				*/
			}
		}
		/*
		_particles->begin();
		int batchSize = 0;
		_particles->begin();
		ZoneTracker z("ParticleManager::render");
		//SpriteVertex v;
		for (int i = 0; i < MAX_PARTICLE_SYSTEMS; ++i) {
			if (_systems[i] != 0) {
				const ParticleArray& array = _systems[i]->getArray();
				const Texture& t = _systems[i]->getTexture();
				if (array.countAlive > 0) {
					for (int j = 0; j < array.countAlive; ++j) {
						_particles->draw(array.position[j].xy(), t, array.rotation[j], array.scale[j], array.color[j]);						
					}
				}
			}
		}
		_particles->end();
		*/
		//renderer::setCurrentShader(renderer::getDefaultShaderID());
	}

	// --------------------------------------------------------------------------
	// debug
	// --------------------------------------------------------------------------
	void ParticleManager::debug() {
		LOG << "---- Particlesystems -----";
		for (size_t i = 0; i < MAX_PARTICLE_SYSTEMS; ++i) {
			if (_systems[i] != 0) {
				LOG << i << " = " << _systems[i]->getDebugName() << " - alive: " << _systems[i]->getCountAlive();
			}
		}
	}
	/*
	void ParticleManager::init(const Descriptor& desc) {
		LOG << "initializing particlemanager";
		BatchBufferDescriptor descriptor;
		descriptor.maxItems = MAX_PARTICLES * 4;
		descriptor.vertexDeclaration = VD_QUAD;
		descriptor.descriptorID = renderer::addDescriptor(desc);
		_particles = new BatchBuffer<ParticleVertex>(descriptor);
	}
	*/
	
}
