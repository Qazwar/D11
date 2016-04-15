#include "Demo.h"
#include "graphics.h"
#include "utils\Log.h"
#include "renderer\render_types.h"
#include "math\math.h"
#include <assert.h>

ds::BaseApp *app = new Demo();

Demo::Demo() : BaseApp() , _timer(0.0f) {	
}

Demo::~Demo() {
	delete _spriteBuffer;
}

bool Demo::initialize() {
	_num = 8 * 16;
	int cnt = 0;
	for (int y = 0; y < 8; ++y) {
		for (int i = 0; i < 16; ++i) {
			ds::Sprite& sp = _sprites[cnt];
			sp.position = v2(200 + i * 60, 100 + y * 60);
			sp.color = Color(255 - i * 14, 0, 0, 255);
			sp.texture = math::buildTexture(0.0f, 340.0f, 46.0f, 42.0f);
			sp.scale = v2(1, 1);
			sp.rotation = 6.28f * (float)cnt / 24.0f;
			++cnt;
		}
	}
	_spriteBuffer = graphics::getSpriteBuffer(8);
	return true;
}

void Demo::update(float dt) {
	for (int i = 0; i < _num; ++i) {
		_sprites[i].rotation += dt;
	}
}

void Demo::render() {
	_spriteBuffer->begin();
	for (int i = 0; i < _num; ++i) {
		_spriteBuffer->draw(_sprites[i]);
	}
	_spriteBuffer->drawText(7, 10, 600, "Hello world!");
	_spriteBuffer->end();
}