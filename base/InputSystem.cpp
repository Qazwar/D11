#include "InputSystem.h"

namespace ds {

	namespace input {

		struct InputContext {
			IDirectInput8* directInput;
			IDirectInputDevice8* keyboard;
			IDirectInputDevice8* mouse;

			unsigned char keyboardState[256];
			DIMOUSESTATE mouseState;

			int screenWidth, screenHeight;
			v2 mousePosition;

			HWND hwnd;
		};

		static InputContext* inputContext = 0;

		bool init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight) {
			inputContext = new InputContext;

			inputContext->screenWidth = screenWidth;
			inputContext->screenHeight = screenHeight;
			inputContext->hwnd = hwnd;
			// Initialize the location of the mouse on the screen.
			inputContext->mousePosition = v2(0, screenHeight);

			// Initialize the main direct input interface.
			HRESULT result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&inputContext->directInput, NULL);
			if (FAILED(result))
			{
				return false;
			}

			// Initialize the direct input interface for the keyboard.
			result = inputContext->directInput->CreateDevice(GUID_SysKeyboard, &inputContext->keyboard, NULL);
			if (FAILED(result))
			{
				return false;
			}

			// Set the data format.  In this case since it is a keyboard we can use the predefined data format.
			result = inputContext->keyboard->SetDataFormat(&c_dfDIKeyboard);
			if (FAILED(result))
			{
				return false;
			}

			// Set the cooperative level of the keyboard to not share with other programs.
			result = inputContext->keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
			if (FAILED(result))
			{
				return false;
			}

			// Now acquire the keyboard.
			result = inputContext->keyboard->Acquire();
			if (FAILED(result))
			{
				return false;
			}

			// Initialize the direct input interface for the mouse.
			result = inputContext->directInput->CreateDevice(GUID_SysMouse, &inputContext->mouse, NULL);
			if (FAILED(result))
			{
				return false;
			}

			// Set the data format for the mouse using the pre-defined mouse data format.
			result = inputContext->mouse->SetDataFormat(&c_dfDIMouse);
			if (FAILED(result))
			{
				return false;
			}


			// Set the cooperative level of the mouse to share with other programs.
			result = inputContext->mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
			if (FAILED(result))
			{
				return false;
			}

			// Acquire the mouse.
			result = inputContext->mouse->Acquire();
			if (FAILED(result))
			{
				return false;
			}

			return true;
		}

		void shutdown() {
			if (inputContext->mouse) {
				inputContext->mouse->Unacquire();
				inputContext->mouse->Release();
				inputContext->mouse = 0;
			}
			if (inputContext->keyboard) {
				inputContext->keyboard->Unacquire();
				inputContext->keyboard->Release();
				inputContext->keyboard = 0;
			}
			if (inputContext->directInput) {
				inputContext->directInput->Release();
				inputContext->directInput = 0;
			}
			delete inputContext;
		}

		bool update() {
			HRESULT result;


			// Read the keyboard device.
			result = inputContext->keyboard->GetDeviceState(sizeof(inputContext->keyboardState), (LPVOID)&inputContext->keyboardState);
			if (FAILED(result))
			{
				// If the keyboard lost focus or was not acquired then try to get control back.
				if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
				{
					inputContext->keyboard->Acquire();
				}
				else
				{
					return false;
				}
			}

			// Read the mouse device.
			result = inputContext->mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&inputContext->mouseState);
			if (FAILED(result))
			{
				// If the mouse lost focus or was not acquired then try to get control back.
				if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
				{
					inputContext->mouse->Acquire();
				}
				else
				{
					return false;
				}
			}

			POINT p;
			if (GetCursorPos(&p)) {
				if (ScreenToClient(inputContext->hwnd, &p)) {
					inputContext->mousePosition.x = p.x;
					inputContext->mousePosition.y = inputContext->screenHeight - p.y;
				}
			}

			/*
			inputContext->mousePosition.x += inputContext->mouseState.lX;
			inputContext->mousePosition.y -= inputContext->mouseState.lY;

			// Ensure the mouse location doesn't exceed the screen width or height.
			if (inputContext->mousePosition.x < 0)  { inputContext->mousePosition.x = 0; }
			if (inputContext->mousePosition.y < 0)  { inputContext->mousePosition.y = 0; }

			if (inputContext->mousePosition.x > inputContext->screenWidth)  { inputContext->mousePosition.x = inputContext->screenWidth; }
			if (inputContext->mousePosition.y > inputContext->screenHeight) { inputContext->mousePosition.y = inputContext->screenHeight; }
			
			//inputContext->mousePosition.y = inputContext->screenHeight - inputContext->mousePosition.y;
			*/
			return true;

		}

		bool getKeyState(int key) {
			if (inputContext->keyboardState[key] & 0x80) {
				return true;
			}
			return false;
		}

		bool isMouseButtonPressed(int button) {
			if (inputContext->mouseState.rgbButtons[0] & 0x80) {
				return true;
			}
			return false;
		}

		v2 getMousePosition() {
			return inputContext->mousePosition;
		}

	}
	
}