#pragma once
#include "world\World.h"

struct Enemy {

	ds::SID sid;
	v2 position;
	v2 velocity;
	int type;
	ds::Texture texture;
};

class Enemies {

public:
	Enemies();
	~Enemies();
	void init();
	void tick(float dt);
	int kill(ds::SID sid);
	v2 getRandomPosition();
private:
	ds::World* _world;
	ds::Array<Enemy> _enemies;
	ds::Texture _textures[4];
};

