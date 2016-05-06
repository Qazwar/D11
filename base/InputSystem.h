#pragma once
#define DIRECTINPUT_VERSION  0x0800
#include <dinput.h>
#include <Vector.h>

namespace ds {

	namespace input {

		bool init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight);

		void shutdown();

		bool update();

		bool getKeyState(int key);

		bool isMouseButtonPressed(int button);

		v2 getMousePosition();
	}

}

