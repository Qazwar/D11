#include "Game.h"

namespace ds {

	Game::Game() {
	}


	Game::~Game() {
		_scenes.destroy_all();
	}

	void Game::render() {
		for (uint32_t i = 0; i < _scenes.size(); ++i) {
			Scene* s = _scenes[i];
			if (s->isActive()) {
				s->draw();
			}
		}
	}

	void Game::update(float dt) {
		for (uint32_t i = 0; i < _scenes.size(); ++i) {
			Scene* s = _scenes[i];
			if (s->isActive()) {
				s->tick(dt);
			}
		}
	}

	Scene2D* Game::create2DScene(const char* name) {
		SceneDescriptor desc;
		Scene2D* scene = new Scene2D(desc);
		IdString hash = string::murmur_hash(name);
		_names.push_back(hash);
		_scenes.push_back(scene);
		return scene;
	}

	void Game::addScene(const char* name, Scene* scene) {
		IdString hash = string::murmur_hash(name);
		_names.push_back(hash);
		_scenes.push_back(scene);
	}

	Scene* Game::getScene(const char* name) const {
		int idx = findSceneIndex(name);
		if (idx != -1) {
			return _scenes[idx];
		}
		return 0;
	}

	void Game::activateScene(const char* name) {
		int idx = findSceneIndex(name);
		if (idx != -1) {
			_scenes[idx]->setActive(true);
		}
	}

	void Game::deactivateScene(const char* name) {
		int idx = findSceneIndex(name);
		if (idx != -1) {
			_scenes[idx]->setActive(false);
		}
	}

	int Game::findSceneIndex(const char* name) const {
		IdString hash = string::murmur_hash(name);
		for (uint32_t i = 0; i < _names.size(); ++i) {
			if (_names[i] == hash) {
				return i;
			}
		}
		return -1;
	}

	void Game::save(const ReportWriter& writer) {
		for (uint32_t i = 0; i < _scenes.size(); ++i) {
			Scene* s = _scenes[i];
			s->save(writer);
		}
	}

}