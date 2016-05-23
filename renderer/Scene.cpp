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
		e.world = matrix::mat4Transform(position);
		e.parent = INVALID_ID;
		e.value = 0;
		return id;
	}

	// ------------------------------------
	// attach
	// ------------------------------------
	void Scene::attach(ID child, ID parent)	{
		if (_entities.contains(child) && _entities.contains(parent)) {
			Entity& e = _entities.get(child);
			e.parent = parent;
		}
	}

	// ------------------------------------
	// draw
	// ------------------------------------
	void Scene::draw() {
		for (EntityList::iterator it = _entities.begin(); it != _entities.end(); ++it) {
			const Entity& e = (*it);
			if (e.active) {
				_meshBuffer->drawImmediate(e.mesh, e.world, e.scale, e.rotation, e.color);
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
	// transform
	// ------------------------------------
	void Scene::transform() {
		for (EntityList::iterator it = _entities.begin(); it != _entities.end(); ++it) {
			Entity& e = (*it);
			if (e.parent == INVALID_ID) {
				e.world = matrix::mat4Transform(e.position);
			}
			else {
				const Entity& parent = _entities.get(e.parent);
				e.world = matrix::mat4Transform(e.position) * parent.world;
			}
		}
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
		// check if this is a parent to anyone and then remove this as well?
		_entities.remove(id);
	}

}