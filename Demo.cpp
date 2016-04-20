#include "Demo.h"
#include "graphics.h"
#include "utils\Log.h"
#include "renderer\render_types.h"
#include "math\math.h"
#include <assert.h>
#include "resources\ResourceContainer.h"
#include "Constants.h"
#include "TestGUI.h"

ds::BaseApp *app = new Demo();

Demo::Demo() : BaseApp(), _timer(0.0f), _firing(false), _fireTimer(0.0f) {
}

Demo::~Demo() {
	delete _enemies;
}

bool Demo::initialize() {
	addGameState(new TestGUIState());
	/*
	_enemies = new Enemies();
	_enemies->init();
	_world = ds::res::getWorld(9);
	_world->enableCollisionChecks();
	_world->setBoundingRect(ds::Rect(10, 10, 1240, 680));
	_world->ignoreCollisions(ObjectTypes::OT_BOMB, ObjectTypes::OT_ENEMY);
	_player = _world->create(v2(640, 60), math::buildTexture(0.0f, 868.0f, 46.0f, 42.0f), ObjectTypes::OT_PLAYER);
	LOG << "Player: " << _player;
	_particles = ds::res::getParticleManager();
	_bombTimer = math::random(2.0f, 3.0f);
	*/
	activate("TestGUIState");
	return true;
}

void Demo::update(float dt) {
	/*
	_particles->update(dt);
	v2 mp;
	if (graphics::getMousePosition(&mp)) {
		mp.y = 60.0f;
		_world->setPosition(_player, mp);
	}
	if (_firing) {
		_fireTimer += dt;
		if (_fireTimer >= 0.4f) {
			_fireTimer -= 0.4f;
			v2 p = _world->getPosition(_player);
			p.y += 10.0f;
			ds::SID id = _world->create(p, math::buildTexture(0.0f, 0.0f, 18.0f, 18.0f), ObjectTypes::OT_BULLET);
			_world->moveBy(id, v2(0, 300));
			_world->attachCollider(id);
			LOG << "bullet: " << id;
		}
	}
	_world->tick(dt);
	if (_world->hasCollisions()) {
		int num = _world->getNumCollisions();
		for (int i = 0; i < num; ++i) {
			const ds::Collision& c = _world->getCollision(i);
			LOG << "c: " << c.firstSID << "/" << ObjectNames[c.firstType] << " " << c.secondSID << "/" << ObjectNames[c.secondType];
			if (c.containsType(ObjectTypes::OT_ENEMY) && c.containsType(ObjectTypes::OT_BULLET)) {
				int points = _enemies->kill(c.getSIDByType(OT_ENEMY));
				if (points > 0) {
					_particles->start(1, c.getPositionByType(OT_ENEMY));
				}
				_world->remove(c.getSIDByType(OT_BULLET));
			}
			if (c.containsType(ObjectTypes::OT_BOMB) && c.containsType(ObjectTypes::OT_PLAYER)) {
				LOG << "player killed";
				_world->remove(c.getSIDByType(OT_BOMB));
			}
		}
	}
	if (_world->hasEvents()) {
		const ds::ActionEventBuffer& buffer = _world->getEventBuffer();
		if (buffer.events.size() > 0) {
			for (int i = 0; i < buffer.events.size(); ++i) {
				if (buffer.events[i].type == ds::AT_MOVE_BY && buffer.events[i].spriteType == ObjectTypes::OT_BULLET) {
					_particles->start(1, _world->getPosition(buffer.events[i].sid));
					_world->remove(buffer.events[i].sid);
				}
				if (buffer.events[i].type == ds::AT_MOVE_BY && buffer.events[i].spriteType == ObjectTypes::OT_BOMB) {
					_particles->start(1, _world->getPosition(buffer.events[i].sid));
					_world->remove(buffer.events[i].sid);
				}
			}
		}
	}
	_enemies->tick(dt);
	_bombTimer += dt;
	if (_bombTimer > 3.0f) {
		_bombTimer = math::random(1.5f, 2.5f);
		v2 p = _enemies->getRandomPosition();
		ds::SID id = _world->create(p, math::buildTexture(16, 56, 24, 24), ObjectTypes::OT_BOMB);
		_world->moveBy(id, v2(0, -200));
		_world->attachCollider(id);
	}
	*/
}

void Demo::render() {
	//_world->render();
	//_particles->render();
}

void Demo::OnButtonDown(int button, int x, int y) {
	//_fireTimer = 0.0f;
	//_firing = true;
}

void Demo::OnButtonUp(int button, int x, int y) {
	//_firing = false;
}

void Demo::OnChar(uint8_t ascii) {
	if (ascii == 'd') {
		ds::ReportWriter rw("reports\\world.html");
		_world->save(rw);
	}
	if (ascii == 'e') {
		_particles->start(1, v2(640, 360));
	}
}