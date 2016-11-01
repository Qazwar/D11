#include "ParticlesTestState.h"
#include "..\renderer\sprites.h"
#include <Vector.h>
#include <core\log\Log.h>
#include <core\math\math.h>

namespace ds {

	ParticlesTestState::ParticlesTestState(const ParticlesTestSettings& settings) : ds::GameState("ParticlesTestState"), _settings(settings) {
		_index = 0;
		_particles = ds::res::getParticleManager();
	}


	ParticlesTestState::~ParticlesTestState() {
	}

	// -------------------------------------------------------
	// init
	// -------------------------------------------------------
	void ParticlesTestState::init() {

	}

	// -------------------------------------------------------
	// activate
	// -------------------------------------------------------
	void ParticlesTestState::activate() {
		_index = 0;
		for (uint32_t i = 0; i < _particles->numSystems(); ++i) {
			const ParticleSystemInfo& info = _particles->getSystemInfo(i);
			ParticleTestEntry entry;
			entry.info = info;
			entry.active = false;
			_entries.push_back(entry);
		}
		LOG << "'e' : Exit";
		LOG << "'+' : Increment index";
		LOG << "'-' : Decrement index";
		LOG << "'s' : Toggle state of selected particle system";
		LOG << "'d' : Debug states";
		LOG << "'1' : Start one of all selected particle systems";
		LOG << "'2' : Start five of all selected particle systems";
	}

	// -------------------------------------------------------
	// dactivate
	// -------------------------------------------------------
	void ParticlesTestState::deactivate() {
	}

	// -------------------------------------------------------
	// Update
	// -------------------------------------------------------
	int ParticlesTestState::update(float dt) {
		return 0;
	}


	// -------------------------------------------------------
	// render
	// -------------------------------------------------------
	void ParticlesTestState::render() {
		if (_settings.render) {
			graphics::selectBlendState(_settings.blendState);
			_particles->render();
		}
		v2 p(10, 710);
		int s = 1;
		gui::start(1, &p);
		gui::begin("ParticleTestState", &s);
		//gui::Header("Select");
		const ParticleTestEntry& current = _entries[_index];
		gui::Label(current.info.name);
		gui::beginGroup();
		if (gui::Button("Next")) {
			++_index;
			if (_index >= _entries.size()) {
				_index = 0;
			}

		}
		if (gui::Button("Prev")) {
			--_index;
			if (_index < 0) {
				_index = _entries.size() - 1;
			}

		}
		if (gui::Button("Toggle")) {
			if (_entries[_index].active) {
				_entries[_index].active = false;
			}
			else {
				_entries[_index].active = true;
			}
		}
		gui::endGroup();
		//gui::Header("Active");
		for (int i = 0; i < _entries.size(); ++i) {
			const ParticleTestEntry& entry = _entries[i];
			if (entry.active) {
				gui::Label(entry.info.name);
			}
		}
		if (gui::Button("Start one")) {
			for (int i = 0; i < _entries.size(); ++i) {
				if (_entries[i].active == 1) {
					float x = _settings.screenSize.x * 0.5f;
					float y = _settings.screenSize.y * 0.5f;
					LOG << "starting " << _entries[i].info.id << " " << _entries[i].info.name;
					_particles->start(_entries[i].info.id, v2(x, y));
				}
			}
		}
		if (gui::Button("Start five")) {
			for (int i = 0; i < 5; ++i) {
				float x = math::random(100.0f, _settings.screenSize.x - 200.0f);
				float y = math::random(100.0f, _settings.screenSize.y - 200.0f);
				for (int j = 0; j < _entries.size(); ++j) {
					if (_entries[j].active == 1) {
						LOG << "starting " << (i + 1) << "/5 " << _entries[j].info.id << " " << _entries[j].info.name;
						_particles->start(_entries[j].info.id, v2(x, y));
					}
				}
			}
		}
		gui::end();
	}
	// -------------------------------------------------------
	// on char
	// -------------------------------------------------------
	int ParticlesTestState::onChar(int ascii) {
		if (ascii == 'e') {
			return 1;
		}
		if (ascii == '+') {
			++_index;
			if (_index >= _entries.size()) {
				_index = 0;
			}
			const ParticleTestEntry& entry = _entries[_index];
			LOG << "index: " << _index << " " << entry.info.name;
		}
		if (ascii == '-') {
			--_index;
			if (_index < 0) {
				_index = _entries.size() - 1;
			}
			const ParticleTestEntry& entry = _entries[_index];
			LOG << "index: " << _index << " " << entry.info.name;
		}
		if (ascii == 's') {
			const ParticleTestEntry& entry = _entries[_index];
			if (_entries[_index].active) {
				_entries[_index].active = false;				
				LOG << "Deactivated " << entry.info.id << " " << entry.info.name;
			}
			else {
				_entries[_index].active = true;
				LOG << "Activated " << entry.info.id << " " << entry.info.name;
			}
		}
		if (ascii == 'd') {
			for (int i = 0; i < _entries.size(); ++i) {
				const ParticleTestEntry& entry = _entries[i];
				LOG << i << " = " << entry.info.id << " " << entry.info.name << " = " << entry.active;
			}
		}
		if (ascii == '1') {
			for (int i = 0; i < _entries.size(); ++i) {
				if (_entries[i].active == 1) {
					float x = _settings.screenSize.x * 0.5f;
					float y = _settings.screenSize.y * 0.5f;
					LOG << "starting " << _entries[i].info.id << " " << _entries[i].info.name;
					_particles->start(_entries[i].info.id, v2(x, y));
				}
			}
		}
		if (ascii == '2') {
			for (int i = 0; i < 5; ++i) {
				float x = math::random(100.0f, _settings.screenSize.x - 200.0f);
				float y = math::random(100.0f, _settings.screenSize.y - 200.0f);
				for (int j = 0; j < _entries.size(); ++j) {
					if (_entries[j].active == 1) {
						LOG << "starting " << (i + 1) << "/5 " << _entries[j].info.id << " " << _entries[j].info.name;
						_particles->start(_entries[j].info.id, v2(x, y));
					}
				}
			}
		}
		return 0;
	}

}