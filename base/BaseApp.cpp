#include "BaseApp.h"
#include "..\graphics.h"
#include "..\utils\Log.h"

BaseApp::BaseApp() {
	_dt = 1.0f / 60.0f;
	_lastTime = GetTickCount();
	_delta = 0;
	_accu = 0.0f;
	_frames = 0;
	_fps = 0;
}


BaseApp::~BaseApp()
{
}


bool BaseApp::prepare(HINSTANCE hInstance, HWND hwnd) {
	return graphics::initialize(hInstance, hwnd);
}

// http://gafferongames.com/game-physics/fix-your-timestep/
void BaseApp::tick() {
	DWORD now = GetTickCount();
	_delta += (now - _lastTime);
	float elapsed = (float)((now - _lastTime) * 0.001);

	_lastTime = now;
	_accu += elapsed;
	if (_delta > 1000) {
		_delta = 0;
		_fps = _frames;
		_frames = 0;
		//LOG << "FPS:" << _fps;
	}
	while (_accu >= _dt) {
		update(_dt);
		_accu -= _dt;
	}
}

void BaseApp::renderFrame() {
	graphics::beginRendering();
	render();
	graphics::endRendering();
	++_frames;
}