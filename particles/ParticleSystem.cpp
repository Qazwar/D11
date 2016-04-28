#include "ParticleSystem.h"
#include "..\utils\Profiler.h"
#include "..\renderer\graphics.h"
#include "..\utils\Log.h"
#include "ParticleManager.h"

namespace ds {

	ParticleSystem::ParticleSystem(int id, const char* name, ParticleSystemFactory* factory) {
		_sendEvents = false;
		strcpy_s(m_DebugName, 32, name);
		sprintf_s(_json_name, 64, "particles\\%s.json", name);
		_id = id;
		m_Array.initialize(1024);
		_count_modules = 0;
		_factory = factory;
		_counter = 0;
	}

	ParticleSystem::~ParticleSystem() {
		clear();
	}
	// -------------------------------------------------
	// start new instance
	// -------------------------------------------------
	ID ParticleSystem::start(const v2& startPosition) {
		if (_spawnerInstances.numObjects < MAX_SPAWNERS) {
			ID id = _spawnerInstances.add();
			ParticleSpawnerInstance& instance = _spawnerInstances.get(id);;
			instance.timer = 0.0f;
			instance.accumulated = 0.0f;
			instance.pos = startPosition;
			instance.loop = _spawner.loop;
			instance.loopTimer = 0.0f;
			instance.loopDelay = _spawner.loopDelay;
			instance.ttl = _spawner.duration;
			uint32_t start = 0;
			uint32_t end = 0;
			{
				ZoneTracker z("PS:emitterGenerate");
				emittParticles(instance, 0.0f, &start, &end);
			}
			return id;
		}
		else {
			LOG << "No more instances available";
			return INVALID_ID;
		}
	}

	// -----------------------------------------------------------
	// get data
	// -----------------------------------------------------------
	ParticleModuleData* ParticleSystem::getData(const char* modifierName) {
		for (int i = 0; i < _count_modules; ++i) {
			const ModuleInstance& instance = _module_instances[i];
			// FIXME: use hash
			if (strcmp(instance.module->getName(), modifierName) == 0) {
				return instance.data;
			}
		}
		return 0;
	}
	
	// -------------------------------------------------
	// tick emitter instances
	// -------------------------------------------------
	void ParticleSystem::updateSpawners(float dt) {
		SpawnerInstances::iterator it = _spawnerInstances.begin();
		while (it != _spawnerInstances.end()) {
			it->timer += dt;

			if (it->ttl > 0.0f && it->timer >= it->ttl) {
				it = _spawnerInstances.remove(it->id);
			}
			else if (it->loop == 0 && it->ttl == 0.0f) {
				it = _spawnerInstances.remove(it->id);
			}
			else {
				++it;
			}
		}
	}

	// -------------------------------------------------------
	// generate
	// -------------------------------------------------------
	void ParticleSystem::emittParticles(ParticleSpawnerInstance& instance, float dt, uint32_t* start, uint32_t* end) {
		if (_spawner.duration > 0.0f) {
			instance.accumulated += _spawner.frequency;
			if (instance.accumulated >= 1.0f) {
				int count = (int)instance.accumulated;
				instance.accumulated -= count;
				emittParticles(instance, count, start, end, dt);
			}
		}
		else if (instance.loop > 0) {
			instance.loopTimer += dt;
			if (instance.loopTimer >= instance.loopDelay) {
				instance.loopTimer = 0.0f;
				emittParticles(instance, _spawner.rate, start, end, dt);
				--instance.loop;
			}
		}
		else {
			emittParticles(instance, _spawner.rate, start, end, dt);
		}
	}

	// -------------------------------------------------------
	// generate
	// -------------------------------------------------------
	void ParticleSystem::emittParticles(const ParticleSpawnerInstance& instance, int count, uint32_t* start, uint32_t* end, float dt) {
		ZoneTracker z("PS:emittParticles");
		*start = m_Array.countAlive;
		*end = *start + count;
		if (*end > m_Array.count) {
			*end = m_Array.count;
		}		
		for (uint32_t i = *start; i < *end; ++i) {
			m_Array.ids[i] = _counter++;
			//LOG << "emitting - index: " << i<< " id: " << m_Array.ids[i];
			m_Array.color[i] = Color::WHITE;
			m_Array.scale[i] = v2(1, 1);
			m_Array.rotation[i] = 0.0f;
			m_Array.timer[i] = v3(0, 1, 1);
			m_Array.forces[i] = v3(0, 0, 0);
			m_Array.position[i] = instance.pos;
		}
		for (uint32_t i = *start; i < *end; ++i) {
			m_Array.wake(i);
		}
		{
			ZoneTracker z("PS:emittParticles:generate");
			for (int i = 0; i < _count_modules; ++i) {
				const ModuleInstance& instance = _module_instances[i];
				void* p = _buffer.get_ptr(i);
				instance.module->generate(&m_Array, instance.data, p, 0.0f, *start, *end);
			}
		}
		//debug();
	}

	// -----------------------------------------------------------
	// update
	// -----------------------------------------------------------
	void ParticleSystem::update(float elapsed, Array<ParticleEvent>& events) {
		ZoneTracker z("PS:update");
		uint32_t start = 0;
		uint32_t end = 0;
		// reset forces
		for (uint32_t i = 0; i < m_Array.countAlive; ++i) {
			m_Array.forces[i] = v2(0, 0);
		}

		updateSpawners(elapsed);
		for (int i = 0; i < _spawnerInstances.numObjects; ++i) {
			ParticleSpawnerInstance& instance = _spawnerInstances.objects[i];
			emittParticles(instance, elapsed, &start, &end);
			if (_sendEvents) {
				for (int i = start; i < end; ++i) {
					ParticleEvent event;
					event.instance = instance.id;
					event.type = ParticleEvent::PARTICLE_EMITTED;
					event.pos = m_Array.position[i].xy();
					events.push_back(event);
				}
			}
		}
		for (int i = 0; i < _count_modules; ++i) {
			const ModuleInstance& instance = _module_instances[i];
			void* p = _buffer.get_ptr(i);
			instance.module->update(&m_Array, instance.data, p, elapsed);
		}

		// kill dead particles
		bool killed = false;
		uint32_t cnt = 0;
		while (cnt < m_Array.countAlive) {
			//if (m_Array.timer[cnt].x > m_Array.timer[cnt].z) {
			if (m_Array.timer[cnt].y >= 1.0f) {
				if (_sendEvents) {
					ParticleEvent event;
					event.instance = INVALID_ID;
					event.type = ParticleEvent::PARTICLE_KILLED;
					event.pos = m_Array.position[cnt].xy();
					events.push_back(event);
				}
				//LOG << "killing at " << cnt << " id: " << m_Array.ids[cnt];
				_buffer.swap(cnt, m_Array.countAlive - 1);
				m_Array.kill(cnt);
				// kill data
				//_buffer.remove(cnt);				
				killed = true;
			}
			++cnt;
		}
		//if (killed) {
			//LOG << "alive: " << m_Array.countAlive;
		//}
		// move particles based on force
		for (uint32_t i = 0; i < m_Array.countAlive; ++i) {
			m_Array.position[i] += m_Array.forces[i] * elapsed;
		}
	}

	void ParticleSystem::debug() {
		LOG << "alive: " << m_Array.countAlive;
		for (int i = 0; i < _count_modules; ++i) {
			const ModuleInstance& instance = _module_instances[i];
			void* p = _buffer.get_ptr(i);
			instance.module->debug(instance.data, p, m_Array.countAlive);
		}
	}

	// -----------------------------------------------------------
	// clear
	// -----------------------------------------------------------
	void ParticleSystem::clear() {
		LOG << "clearing particlesystem: " << m_DebugName;
		for (int i = 0; i < _count_modules; ++i) {
			const ModuleInstance& instance = _module_instances[i];
			if (instance.data != 0) {
				delete instance.data;
			}
		}		
		_count_modules = 0;
		_spawnerInstances.clear();
	}

	// -----------------------------------------------------------
	// save data
	// -----------------------------------------------------------
	bool ParticleSystem::saveData(JSONWriter& writer) {
		/*
		writer.startCategory("particlesystem");
		writer.write("max", 1024);
		writer.write("texture_id", _system_data.textureID);
		writer.write("texture_rect", _system_data.texture.rect);
		writer.startCategory("emitter");
		writer.write("count", _emitter_data.count);
		writer.write("ejection_period", _emitter_data.ejectionPeriod);
		writer.write("ejection_variance", _emitter_data.ejectionVariance);
		writer.write("ejection_counter", _emitter_data.ejectionCounter);
		writer.endCategory();
		writer.startCategory("generators");
		for (int i = 0; i < _count_generators; ++i) {
			const GeneratorInstance& instance = _generator_instances[i];
			writer.startCategory(instance.generator->getName());
			if (instance.data != 0) {
				instance.data->save(writer);
			}
			writer.endCategory();
		}
		writer.endCategory();
		writer.startCategory("modifiers");
		for (int i = 0; i < _count_modifiers; ++i) {
			const ModifierInstance& instance = _modifier_instances[i];
			writer.startCategory(instance.modifier->getName());
			if (instance.data != 0) {
				instance.data->save(writer);
			}
			writer.endCategory();
		}
		writer.endCategory();
		writer.endCategory();
		*/
		return true;
	}

	// -----------------------------------------------------------
	// load data
	// -----------------------------------------------------------
	bool ParticleSystem::loadData(const JSONReader& reader) {
		LOG << "importing data";
		clear();		
		int em_id = reader.find_category("spawn");
		if (em_id != -1) {
			reader.get_float(em_id, "duration", &_spawner.duration);
			reader.get_int(em_id, "rate", &_spawner.rate);
			reader.get_int(em_id, "loop", &_spawner.loop);
			reader.get_float(em_id, "loop_delay", &_spawner.loopDelay);
			Rect r;
			reader.get(em_id, "texture", &r);
			_texture = math::buildTexture(r);
			initSpawner();
		}
		else {
			LOGE << "cannot find spawn data - required!";
			return false;
		}
		int children[MAX_SPAWNERS];
		int num = reader.get_categories(children, MAX_SPAWNERS);
		for (int i = 0; i < num; ++i) {
			if (children[i] != em_id) {
				const char* mod_name = reader.get_category_name(children[i]);
				if (_factory->addModule(this, mod_name)) {
					ParticleModuleData* data = getData(mod_name);
					if (data != 0) {
						data->read(reader, children[i]);
					}
				}
				else {
					LOGE << "cannot find module: " << mod_name;
				}
			}
		}			
		LOG << "init buffer - sizes: " << _count_modules;
		for (int i = 0; i < _count_modules; ++i) {
			LOG << i << " : " << _sizes[i];
		}
		_buffer.init(_sizes, _count_modules);
		_buffer.resize(1024);
		return true;
	}

	// -----------------------------------------------------------
	// prepare vertices
	// -----------------------------------------------------------
	void ParticleSystem::prepareVertices() {
		/*
		for (int i = 0; i < MAX_PARTICLES; ++i) {
			m_Array.vertices[i * 4].uv.x = _texture.uv.x;
			m_Array.vertices[i * 4].uv.y = _texture.uv.y;
			m_Array.vertices[i * 4 + 1].uv.x = _texture.uv.z;
			m_Array.vertices[i * 4 + 1].uv.y = _texture.uv.y;
			m_Array.vertices[i * 4 + 2].uv.x = _texture.uv.z;
			m_Array.vertices[i * 4 + 2].uv.y = _texture.uv.w;
			m_Array.vertices[i * 4 + 3].uv.x = _texture.uv.x;
			m_Array.vertices[i * 4 + 3].uv.y = _texture.uv.w;
		}
		*/
	}

	// -----------------------------------------------------------
	// init emitter data
	// -----------------------------------------------------------
	void ParticleSystem::initSpawner() {
		_spawner.frequency = 0.0f;
		if (_spawner.duration > 0.0f) {
			_spawner.frequency = static_cast<float>(_spawner.rate) / (60.0f * _spawner.duration);
		}
		LOG << "--> frequency: " << _spawner.frequency;
	}

	// -----------------------------------------------------------
	//  get modifier names
	// -----------------------------------------------------------
	void ParticleSystem::getModuleNames(Array<const char*>& names) {
		names.clear();
		for (int i = 0; i < _count_modules; ++i) {
			names.push_back(_module_instances[i].module->getName());
		}
	}


}
