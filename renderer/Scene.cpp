#include "Scene.h"
#include "..\resources\ResourceContainer.h"

namespace ds {

	Scene::Scene(const char* meshBufferName) {
		_meshBuffer = res::getMeshBuffer(meshBufferName);
	}

	Scene::~Scene()	{
	}

	// ------------------------------------
	// add entity
	// ------------------------------------
	ID Scene::add(const char* meshName, const v3& position) {
		ID id = _entities.add();
		Entity& e = _entities.get(id);
		e.mesh = res::getMesh(meshName);
		e.position = position;
		e.scale = v3(1, 1, 1);
		e.rotation = v3(0, 0, 0);
		e.timer = 0.0f;
		e.active = true;
		e.type = -1;
		return id;
	}

	// ------------------------------------
	// draw
	// ------------------------------------
	void Scene::draw() {
		for (EntityList::iterator it = _entities.begin(); it != _entities.end(); ++it) {
			const Entity& e = (*it);
			if (e.active) {
				_meshBuffer->drawImmediate(e.mesh, e.position, e.scale, e.rotation);
			}
		}
	}

	// ------------------------------------
	// find entities by type
	// ------------------------------------
	int Scene::find(int type, ID* ids, int max) {
		int cnt = 0;
		for (EntityList::iterator it = _entities.begin(); it != _entities.end(); ++it) {
			const Entity& e = (*it);
			if (e.type == type && cnt < max) {
				ids[cnt++] = e.id;
			}
		}
		return cnt;
	}

	// ------------------------------------
	// get entity
	// ------------------------------------
	Entity& Scene::get(ID id) {
		return _entities.get(id);
	}

	// ------------------------------------
	// get entity
	// ------------------------------------
	const Entity& Scene::get(ID id) const {
		return _entities.get(id);
	}

	// ------------------------------------
	// remove entity
	// ------------------------------------
	void Scene::remove(ID id) {
		_entities.remove(id);
	}

}