#include "Enemies.h"
#include "resources\ResourceContainer.h"
#include "math\math.h"
#include "Constants.h"

Enemies::Enemies() {
	_world = ds::res::getWorld(9);
	_textures[0] = math::buildTexture(0.0f, 340.0f, 46.0f, 42.0f);
	_textures[1] = math::buildTexture(0.0f, 388.0f, 46.0f, 42.0f);
	_textures[2] = math::buildTexture(0.0f, 436.0f, 46.0f, 42.0f);
	_textures[3] = math::buildTexture(0.0f, 484.0f, 46.0f, 42.0f);
}


Enemies::~Enemies() {
}

void Enemies::init() {
	for (int y = 0; y < 5; ++y) {
		int type = math::random(0.0f, 3.99f);
		for (int i = 0; i < 11; ++i) {
			Enemy e;
			v2 position = v2(200 + i * 60, 400 + y * 60);
			const ds::Texture& t = _textures[type];
			e.sid = _world->create(position, t, ObjectTypes::OT_ENEMY);
			LOG << "enemy: " << e.sid;
			e.texture = t;
			e.type = type;
			e.position = position;
			e.velocity = v2(100, 0);
			_world->attachCollider(e.sid);
			_enemies.push_back(e);
		}
	}
}

void Enemies::tick(float dt) {
	bool flip = false;
	for (int i = 0; i < _enemies.size(); ++i) {
		const Enemy& e = _enemies[i];
		if (e.sid != ds::INVALID_SID) {
			v2 p = _world->getPosition(e.sid);
			p += e.velocity * dt;
			_world->setPosition(e.sid, p);
			if (p.x > 1100.0f || p.x < 200.0f) {
				flip = true;
			}
		}
	}
	if (flip) {
		for (int i = 0; i < _enemies.size(); ++i) {
			Enemy& e = _enemies[i];
			if (e.sid != ds::INVALID_SID) {
				v2 p = _world->getPosition(e.sid);
				e.velocity *= -1.0f;
				p.y -= 21.0f;
				_world->setPosition(e.sid, p);
			}
		}
	}
}

int Enemies::kill(ds::SID sid) {
	int killed = 0;
	for (int i = 0; i < _enemies.size(); ++i) {
		const Enemy& e = _enemies[i];
		if (e.sid == sid) {		
			int type = e.type;
			LOG << "removing at: " << i;
			_world->remove(e.sid);
			_enemies.remove(i);
			return type;
		}
	}
	/*
	if (killed - 55 < 4) {
		for (int i = 0; i < _enemies.size(); ++i) {
			Enemy& e = _enemies[i];
			if (e.sid != ds::INVALID_SID) {
				e.velocity *= 2.0f;
			}
		}
	}
	*/
	return -1;

}

v2 Enemies::getRandomPosition() {
	int idx = math::random(0, _enemies.size() - 1);
	ds::SID sid = _enemies[idx].sid;
	return _world->getPosition(sid);
}
