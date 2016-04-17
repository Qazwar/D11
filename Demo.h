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
	void OnButtonDown(int button, int x, int y);
	void OnButtonUp(int button, int x, int y);
private:
	ds::World* _world;
	ds::SID _player;
	Enemies* _enemies;
	float _timer;
	int _num;
	bool _firing;
	float _fireTimer;
};

