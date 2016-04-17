#include "Demo.h"
#include "graphics.h"
#include "utils\Log.h"
#include "renderer\render_types.h"
#include "math\math.h"
#include <assert.h>
#include "resources\ResourceContainer.h"

ds::BaseApp *app = new Demo();

Demo::Demo() : BaseApp() , _timer(0.0f) {	
}

Demo::~Demo() {
	delete _enemies;
}

bool Demo::initialize() {
	_enemies = new Enemies();
	_enemies->init();
	_world = ds::res::getWorld(9);
	return true;
}

void Demo::update(float dt) {
	_world->tick(dt);
	_enemies->tick(dt);
}

void Demo::render() {
	_world->render();
}