#include "Demo.h"
#include "graphics.h"
#include "utils\Log.h"
#include "renderer\render_types.h"
#include "math\math.h"
#include <assert.h>
#include "resources\ResourceContainer.h"

ds::BaseApp *app = new Demo();

Demo::Demo() : BaseApp(), _timer(0.0f), _firing(false), _fireTimer(0.0f) {
}

Demo::~Demo() {
	delete _enemies;
}

bool Demo::initialize() {
	_enemies = new Enemies();
	_enemies->init();
	_world = ds::res::getWorld(9);
	_world->enableCollisionChecks();
	_player = _world->create(v2(640, 60), math::buildTexture(0.0f, 868.0f, 46.0f, 42.0f),1);
	return true;
}

void Demo::update(float dt) {
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
			ds::SID id = _world->create(p, math::buildTexture(0.0f, 0.0f, 18.0f, 18.0f),0);
			_world->moveBy(id, v2(0, 300));
			_world->attachCollider(id);
		}
	}
	_world->tick(dt);
	if (_world->hasCollisions()) {
		int num = _world->getNumCollisions();
		for (int i = 0; i < num; ++i) {
			const ds::Collision& c = _world->getCollision(i);
			LOG << "c: " << c.firstSID << "/" << c.firstType << " " << c.secondSID << "/" << c.secondType;
			if (c.containsType(2)) {
				int points = _enemies->kill(c.getSIDByType(2));
			}
			if (c.containsType(0)) {
				_world->remove(c.getSIDByType(0));
			}
		}
	}
	_enemies->tick(dt);
}

void Demo::render() {
	_world->render();
}

void Demo::OnButtonDown(int button, int x, int y) {
	_fireTimer = 0.0f;
	_firing = true;
}

void Demo::OnButtonUp(int button, int x, int y) {
	_firing = false;
}