#pragma once
#include <Windows.h>
#include <Vector.h>

namespace ds {

	namespace input {

		bool init(HWND hwnd, int screenWidth, int screenHeight);

		void shutdown();

		bool update(RAWINPUT* raw);
		
		bool getKeyState(int key);

		bool isMouseButtonPressed(int button);

		v2 getMousePosition();

		
	}

}

