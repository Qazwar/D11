#pragma once
#include <Windows.h>
#include "Settings.h"
#include "..\resources\ResourceContainer.h"

namespace ds {

	class BaseApp {

	public:
		BaseApp();
		~BaseApp();
		bool prepare(HINSTANCE hInstance, HWND hwnd);
		virtual bool initialize() = 0;
		virtual void render() = 0;
		virtual void update(float dt) = 0;
		void tick();
		void renderFrame();
	private:
		float _dt;
		DWORD _lastTime;
		DWORD _delta;
		float _accu;
		int _frames;
		int _fps;
		Settings _settings;
		ResourceContainer* _resources;
	};

}