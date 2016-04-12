#include "Demo.h"
#include "graphics.h"
#include "utils\Log.h"
#include "renderer\render_types.h"
#include "math\math.h"
#include <assert.h>


Demo::Demo() {
	_num = 18;
	for (int i = 0; i < 18; ++i) {
		_positions[i] = v2(40 + i * 70, 360);
	}
}

Demo::~Demo() {
}

bool Demo::initialize() {
	_spriteBuffer = new SpriteBuffer(1024);
	return true;
}

void Demo::shutdown() {
	delete _spriteBuffer;
}

void Demo::render() {
	_spriteBuffer->begin();
	for (int i = 0; i < _num; ++i) {
		_spriteBuffer->draw(_positions[i], math::buildTexture(70.0f, 0.0f, 60.0f, 60.0f));
	}
	_spriteBuffer->end();
}