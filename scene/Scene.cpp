#include "Scene.h"
#include "..\resources\ResourceContainer.h"
#include "actions\ScalingAction.h"
#include "actions\MoveToAction.h"
#include "actions\RotateToAction.h"

namespace ds {

	Scene::Scene(const SceneDescriptor& descriptor) : _descriptor(descriptor) , _active(false) {
		//_meshBuffer = res::getMeshBuffer(descriptor.meshBuffer);
		//_camera = graphics::getFPSCamera();
		_depthEnabled = descriptor.depthEnabled;
		for (size_t i = 0; i < MAX_ACTIONS; ++i) {
			_actions[i] = 0;
		}
		_actions[AT_SCALE] = new ScalingAction;
		_actions[AT_MOVE_TO] = new MoveToAction;
		_actions[AT_ROTATE_TO] = new RotateToAction;

	}

	Scene::~Scene()	{
		if (_data.buffer != 0) {
			DEALLOC(_data.buffer);
		}
		for (size_t i = 0; i < MAX_ACTIONS; ++i) {
			if (_actions[i] != 0) {
				delete _actions[i];
			}
		}
	}

	// ------------------------------------
	// add entity
	// ------------------------------------
	ID Scene::add(const char* meshName, const v3& position, RID material, DrawMode mode) {
		Mesh* m = res::getMesh(meshName);
		return add(m, position, material);
	}
	
	// ------------------------------------
	// add entity
	// ------------------------------------
	ID Scene::add(Mesh* mesh, const v3& position, RID material, DrawMode mode) {
		return _data.create(position, mesh, v3(1, 1, 1), v3(0, 0, 0), material, Color::WHITE);
		/*
		ID id = _entities.add();
		Entity& e = _entities.get(id);
		e.mesh = mesh;
		e.position = position;
		e.scale = v3(1, 1, 1);
		e.rotation = v3(0, 0, 0);
		e.timer = 0.0f;
		e.active = true;
		e.type = -1;
		e.world = matrix::mat4Transform(position);
		e.parent = INVALID_ID;
		e.value = 0;
		e.mode = mode;
		e.material = material;
		return id;
		*/
	}

	// ------------------------------------
	// add entity
	// ------------------------------------
	ID Scene::addStatic(Mesh* mesh, const v3& position, RID material) {
		ID id = _data.create(position, mesh, v3(1, 1, 1), v3(0, 0, 0), material, Color::WHITE);
		_data.setDrawMode(id, DrawMode::STATIC);
		_data.setStaticIndex(id, _staticMeshes.size());
		const mat4& world = _data.getWorld(id);
		/*
		ID id = _entities.add();
		Entity& e = _entities.get(id);
		e.mesh = mesh;
		e.position = position;
		e.scale = v3(1, 1, 1);
		e.rotation = v3(0, 0, 0);
		e.timer = 0.0f;
		e.active = true;
		e.type = -1;
		e.world = matrix::mat4Transform(position);
		e.parent = INVALID_ID;
		e.value = 0;
		e.mode = STATIC;
		e.staticIndex = _staticMeshes.size();
		e.material = material;
		*/
		StaticMesh sm;
		sm.id = id;
		sm.index = _staticVertices.size();
		sm.size = mesh->vertices.size();
		for (int i = 0; i < mesh->vertices.size(); ++i) {
			PNTCVertex v = mesh->vertices[i];
			v.position = world * v.position;
			_staticVertices.push_back(v);
		}
		_staticMeshes.push_back(sm);
		return id;
	}

	void Scene::updateWorld(int idx) {
		mat4 rotY = matrix::mat4RotationY(_data.rotations[idx].y);
		mat4 rotX = matrix::mat4RotationX(_data.rotations[idx].x);
		mat4 rotZ = matrix::mat4RotationZ(_data.rotations[idx].z);
		mat4 t = matrix::mat4Transform(_data.positions[idx]);
		mat4 s = matrix::mat4Scale(_data.scales[idx]);
		_data.worlds[idx] = rotZ * rotY * rotX * s * t;
		_data.dirty[idx] = false;
		if (_data.parents[idx] != INVALID_ID) {
			int pidx = _data.getIndex(_data.parents[idx]);
			_data.worlds[idx] = _data.worlds[idx] * _data.worlds[pidx];
		}
	}

	void Scene::activate(ID id) {
		if (_data.contains(id)) {
			int idx = _data.getIndex(id);
			_data.active[idx] = true;
		}
	}

	bool Scene::isActive(ID id) const {
		if (_data.contains(id)) {
			int idx = _data.getIndex(id);
			return _data.active[idx];
		}
		return false;
	}

	void Scene::setPosition(ID id, const v3& p) {
		if (_data.contains(id)) {
			int idx = _data.getIndex(id);
			_data.positions[idx] = p;
			_data.dirty[idx] = true;
		}
	}

	void Scene::setColor(ID id, const Color& clr) {
		if (_data.contains(id)) {
			int idx = _data.getIndex(id);
			_data.colors[idx] = clr;
		}
	}

	void Scene::deactivate(ID id) {
		if (_data.contains(id)) {
			int idx = _data.getIndex(id);
			_data.active[idx] = false;
		}
	}

	void Scene::rotate(ID id, const v3& r) {
		if (_data.contains(id)) {
			int idx = _data.getIndex(id);
			_data.rotations[idx] = r;
			_data.dirty[idx] = true;
		}
	}

	// ------------------------------------
	// attach
	// ------------------------------------
	void Scene::attach(ID child, ID parent)	{
		if (_data.contains(child)) {
			int idx = _data.getIndex(child);
			_data.parents[idx] = parent;
		}
	}

	void Scene::clear() {
		_data.clear();
	}
	// ------------------------------------
	// draw
	// ------------------------------------
	void Scene::draw() {
		ZoneTracker z("Scene::draw");
		_currentMaterial = INVALID_RID;
		graphics::setCamera(_camera);
		if (_depthEnabled) {
			graphics::turnOnZBuffer();
		}
		else {
			graphics::turnOffZBuffer();
		}
		_meshBuffer->begin();
		for (int i = 0; i < _data.num; ++i) {
			if (_data.active[i]) {
				if (_data.materials[i] != _currentMaterial) {
					_meshBuffer->flush();
					_currentMaterial = _data.materials[i];
				}
				if (_data.drawModes[i] == DrawMode::IMMEDIATE) {
					_meshBuffer->flush();
					_meshBuffer->drawImmediate(_data.meshes[i], _data.worlds[i], _data.scales[i], _data.rotations[i], _data.colors[i]);
				}
				else if (_data.drawModes[i] == DrawMode::TRANSFORM) {
					_meshBuffer->add(_data.meshes[i], _data.worlds[i], _data.colors[i]);
				}
				else if (_data.drawModes[i] == DrawMode::STATIC) {
					const StaticMesh& sm = _staticMeshes[_data.staticIndices[i]];
					_meshBuffer->add(_staticVertices.data() + sm.index, sm.size);
				}
			}
		}
		_meshBuffer->end();
	}

	// ------------------------------------
	// find entities by type
	// ------------------------------------
	int Scene::find(int type, ID* ids, int max) {
		int cnt = 0;
		/*
		for (EntityList::iterator it = _entities.begin(); it != _entities.end(); ++it) {
			const Entity& e = (*it);
			if (e.type == type && cnt < max) {
				ids[cnt++] = e.id;
			}
		}
		*/
		return cnt;
	}
	
	// ------------------------------------
	// remove entity
	// ------------------------------------
	void Scene::remove(ID id) {
		// check if this is a parent to anyone and then remove this as well?
		_data.remove(id);
	}

	// ------------------------------------
	// intersects with ray
	// ------------------------------------
	ID Scene::intersects(const Ray& ray) {
		float t0 = 0.0f;
		float t1 = 0.0f;
		ID ret = INVALID_ID;
		float tm = 0.0f;
		for ( int i = 0; i < _data.num; ++i ) {
			if (_data.active[i]) {
				AABBox bb = _data.meshes[i]->boundingBox;
				bb.transpose(_data.positions[i]);
				if (bb.intersects(ray, &t0, &t1)) {
					if (ret == INVALID_ID) {
						tm = t0;
						ret = _data.ids[i];
					}
					else {
						if (t0 < tm) {
							tm = t0;
							ret = _data.ids[i];
						}
					}
				}
			}
		}
		/*
		if (ret != INVALID_ID) {
			LOG << "INTERSECTION!!!! id: " << ret << " t0: " << t0 << " t1: " << t1;
		}
		else {
			LOG << "NO intersection!";
		}
		*/
		return ret;
	}

	// ------------------------------------
	// tick
	// ------------------------------------
	void Scene::tick(float dt) {
		ZoneTracker z("World:tick");
		_eventBuffer.reset();
		{
			ZoneTracker z1("World:tick:actions");
			for (size_t i = 0; i < MAX_ACTIONS; ++i) {
				if (_actions[i] != 0) {
					_actions[i]->update(_data, dt, _eventBuffer);
				}
			}
		}
		{
			ZoneTracker z2("World:tick:transform");
			for (int i = 0; i < _data.num; ++i) {
				if (_data.active[i] && _data.dirty[i]) {					
					updateWorld(i);
				}
			}
		}
		{
			ZoneTracker z2("World:tick:kill");
			for (int i = 0; i < _eventBuffer.events.size(); ++i) {
				const ActionEvent& e = _eventBuffer.events[i];
				if (e.type == AT_KILL) {
					remove(e.id);
				}
			}
		}
		//if (_checkCollisions) {
			//m_Physics.tick(&m_Data, dt);
		//}
	}

	// ------------------------------------
	// save report
	// ------------------------------------
	void Scene::save(const ReportWriter& writer) {
		char buffer[256];
		//sprintf_s(buffer, 256, "Scene - %s", res::getName(_descriptor.id));
		//writer.startBox(buffer);
		const char* HEADERS[] = { "ID", "Pos", "Scale", "Rotation" };
		writer.startTable(HEADERS, 4);
		for (uint32_t i = 0; i < _data.num; ++i) {
			writer.startRow();
			writer.addCell(_data.ids[i]);
			writer.addCell(_data.positions[i]);
			writer.addCell(_data.scales[i]);
			writer.addCell(_data.rotations[i]);
			writer.endRow();
		}
		writer.endTable();		
		for (size_t i = 0; i < MAX_ACTIONS; ++i) {
			if (_actions[i] != 0) {
				_actions[i]->save(writer);
			}
		}
		writer.endBox();
	}

	// ------------------------------------
	// scale to
	// ------------------------------------
	void Scene::scaleTo(ID id, const v3& startScale, const v3& endScale, float ttl, int mode, const tweening::TweeningType& tweeningType) {
		ScalingAction* action = (ScalingAction*)_actions[AT_SCALE];
		action->attach(id, startScale, endScale, ttl, mode, tweeningType);
	}

	// ------------------------------------
	// move to
	// ------------------------------------
	void Scene::moveTo(ID id, const v3& startPos, const v3& endPos, float ttl, int mode, const tweening::TweeningType& tweeningType) {
		MoveToAction* action = (MoveToAction*)_actions[AT_MOVE_TO];
		action->attach(id, _data, startPos, endPos, ttl, mode, tweeningType);
	}

	// ------------------------------------
	// rotate to
	// ------------------------------------
	void Scene::rotateTo(ID id, const v3& startRotation, const v3& endRotation, float ttl, int mode, const tweening::TweeningType& tweeningType) {
		RotateToAction* action = (RotateToAction*)_actions[AT_ROTATE_TO];
		action->attach(id, startRotation, endRotation, ttl, mode, tweeningType);
	}



	// ------------------------------------
	// Scene2D
	// ------------------------------------

	// ------------------------------------
	// add
	// ------------------------------------
	ID Scene2D::add(const v2& pos, const Texture& t, RID material) {
		return _data.create(pos, t, v2(1, 1), 0.0f, material, Color::WHITE);
	}

	// ------------------------------------
	// draw
	// ------------------------------------
	void Scene2D::draw() {
		SpriteBuffer* sprites = graphics::getSpriteBuffer();
		sprites->begin();
		for (int i = 0; i < _data.num; ++i) {
			if (_data.active[i]) {
				sprites->draw(_data.positions[i].xy(), _data.textures[i],_data.rotations[i].z,_data.scales[i].xy());
			}
		}
		sprites->end();
	}

	// ------------------------------------
	// scale to
	// ------------------------------------
	void Scene2D::scaleTo(ID id, const v2& startScale, const v2& endScale, float ttl, int mode, const tweening::TweeningType& tweeningType) {
		ScalingAction* action = (ScalingAction*)_actions[AT_SCALE];
		action->attach(id, v3(startScale,0.0f), v3(endScale,0.0f), ttl, mode, tweeningType);
	}

	void Scene2D::scale(ID id, const v2& scale) {
		int idx = _data.getIndex(id);
		_data.scales[idx] = v3(scale, 0.0f);
	}

	void Scene2D::setTexture(ID id, const Texture& t) {
		int idx = _data.getIndex(id);
		_data.textures[idx] = t;
	}
}