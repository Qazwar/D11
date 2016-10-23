#include "ParticlesTestState.h"
#include "..\renderer\sprites.h"
#include <Vector.h>
#include <core\log\Log.h>
#include <core\math\math.h>

namespace ds {

	ParticlesTestState::ParticlesTestState(const ParticlesTestSettings& settings) : ds::GameState("ParticlesTestState"), _settings(settings) {
		_index = 0;
		_num = 0;
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
		_num = _particles->getSystemIds(_ids, 64);
		for (int i = 0; i < 64; ++i) {
			_states[i] = 0;
		}
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
			if (_index > _num) {
				_index = 0;
			}
			LOG << "index: " << _index;
		}
		if (ascii == '-') {
			--_index;
			if (_index < 0) {
				_index = _num - 1;
			}
			LOG << "index: " << _index;
		}
		if (ascii == 's') {
			if (_states[_index] == 1) {
				_states[_index] = 0;
			}
			else {
				_states[_index] = 1;
			}
		}
		if (ascii == 'd') {
			for (int i = 0; i < _num; ++i) {
				LOG << i << " = " << _states[i];
			}
		}
		if (ascii == '1') {
			for (int i = 0; i < _num; ++i) {
				if (_states[i] == 1) {
					float x = _settings.screenSize.x * 0.5f;
					float y = _settings.screenSize.y * 0.5f;
					_particles->start(_ids[i], v2(x, y));
				}
			}
		}
		if (ascii == '2') {
			for (int i = 0; i < 5; ++i) {
				float x = math::random(100.0f, _settings.screenSize.x - 200.0f);
				float y = math::random(100.0f, _settings.screenSize.y - 200.0f);
				for (int j = 0; j < _num; ++j) {
					if (_states[j] == 1) {
						_particles->start(_ids[j], v2(x, y));
					}
				}
			}
		}
		return 0;
	}

}