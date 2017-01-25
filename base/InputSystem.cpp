#include "InputSystem.h"
#include <Windows.h>
#include "core\log\Log.h"
#include "core\base\EventStream.h"

namespace ds {

	namespace input {

		struct InputContext {

			unsigned char keyboardState[256];
			int screenWidth, screenHeight;
			int mouseButtonState[6];
			HWND hwnd;

		};

		static InputContext* inputContext = 0;

		bool init(HWND hwnd, int screenWidth, int screenHeight) {
			inputContext = new InputContext;
			inputContext->hwnd = hwnd;
			inputContext->screenWidth = screenWidth;
			inputContext->screenHeight = screenHeight;
			for (int i = 0; i < 6; ++i) {
				inputContext->mouseButtonState[i] = 0;
			}
			return true;
		}

		void shutdown() {
			delete inputContext;
		}

		bool update(RAWINPUT* raw) {
			HWND current = GetForegroundWindow();
			if (current == inputContext->hwnd) {
				if (raw->header.dwType == RIM_TYPEKEYBOARD) {
					const RAWKEYBOARD& rawKB = raw->data.keyboard;
					UINT virtualKey = rawKB.VKey;
					UINT scanCode = rawKB.MakeCode;
					UINT flags = rawKB.Flags;
					if (flags == 0) {
						inputContext->keyboardState[virtualKey] = 80;
					}
					else {
						inputContext->keyboardState[virtualKey] = 0;

					}
				}
				if (raw->header.dwType == RIM_TYPEMOUSE) {
					//if (raw->data.mouse.ulButtons > 0) {
						//LOG << "flags: " << raw->data.mouse.usFlags << " buttons: " << raw->data.mouse.ulButtons << " button flags: " << raw->data.mouse.usButtonFlags << " button data: " << raw->data.mouse.usButtonData << " button raw: " << raw->data.mouse.ulRawButtons << " button lastX: " << raw->data.mouse.lLastX << " button lastY: " << raw->data.mouse.lLastY;
					//}
					if (raw->data.mouse.ulButtons == 1) {
						events::send(events::SE_BUTTON_ONE_DOWN);
						inputContext->mouseButtonState[0] = 1;
					}
					if (raw->data.mouse.ulButtons == 2) {
						events::send(events::SE_BUTTON_ONE_UP);
						inputContext->mouseButtonState[0] = 0;
					}
					if (raw->data.mouse.ulButtons == 4) {
						inputContext->mouseButtonState[1] = 1;
					}
					if (raw->data.mouse.ulButtons == 8) {
						inputContext->mouseButtonState[1] = 0;
					}
				}
			}
			return true;

		}

		bool getKeyState(int key) {
			if (inputContext->keyboardState[key] == 80 ) {
				return true;
			}
			return false;
		}

		bool isMouseButtonPressed(int button) {
			return inputContext->mouseButtonState[button] == 1;
		}

		v2 getMousePosition() {
			v2 mp;
			POINT p;
			if (GetCursorPos(&p)) {
				if (ScreenToClient(inputContext->hwnd, &p)) {
					mp.x = p.x;
					mp.y = inputContext->screenHeight - p.y;
				}
			}
			return mp;
		}

	}
	
}