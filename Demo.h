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

class Demo : public BaseApp {

public:
	Demo();
	~Demo();
	bool initialize();
	void render();
	void update(float dt);
private:
	SpriteBuffer* _spriteBuffer;
	v2 _positions[256];
	Color _colors[256];
	int _num;
};

