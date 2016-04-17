#pragma once
#include <stdint.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <DxErr.h>
#include <xnamath.h>
#include <Vector.h>
#include "renderer\sprites.h"
#include "utils\Color.h"
#include "base\BaseApp.h"
#include "world\World.h"
#include "Enemies.h"

class Demo : public ds::BaseApp {

public:
	Demo();
	~Demo();
	bool initialize();
	void render();
	void update(float dt);
private:
	ds::World* _world;
	Enemies* _enemies;
	float _timer;
	int _num;
};

