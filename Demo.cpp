#include "Demo.h"
#include "graphics.h"
#include "utils\Log.h"
#include "renderer\render_types.h"
#include "math\math.h"
#include <assert.h>

BaseApp *app = new Demo();

Demo::Demo() : BaseApp() , _timer(0.0f) {	
}

Demo::~Demo() {
	delete _spriteBuffer;
}

bool Demo::initialize() {
	_num = 24;
	int cnt = 0;
	for (int y = 0; y < 4; ++y) {
		for (int i = 0; i < 6; ++i) {
			Sprite& sp = _sprites[cnt];
			sp.position = v2(400 + i * 100, 200 + y * 100);
			sp.color = Color(255 - i * 14, 0, 0, 255);
			sp.texture = math::buildTexture(70.0f, 0.0f, 60.0f, 60.0f);
			sp.scale = v2(1, 1);
			sp.rotation = 6.28f * (float)cnt / 24.0f;
			++cnt;
		}
	}
	_spriteBuffer = new SpriteBuffer(1024);
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
	_spriteBuffer->end();
}