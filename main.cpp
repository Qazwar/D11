#include <Windows.h>
#include "graphics.h"
#include "Demo.h"
#include "utils\Log.h"
#include "base\BaseApp.h"

extern BaseApp* app;

LRESULT CALLBACK WinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT paintStruct;
	HDC hDC;

	switch (message) {
		case WM_PAINT:
			hDC = BeginPaint(hwnd, &paintStruct);
			EndPaint(hwnd, &paintStruct);
			break;
		case WM_DESTROY :
			PostQuitMessage(0);
			break;
		}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hThisInst, HINSTANCE hLastInst, LPSTR lpszCmdLine, int nCmdShow) {
	WNDCLASSEX wndClass = { 0 };
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WinProc;
	wndClass.hInstance = hThisInst;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = "Diesel11";

	if (!RegisterClassEx(&wndClass)) {
		return -1;
	}
	RECT rc = { 0, 0, 1280, 720};
	AdjustWindowRect(&rc, WS_OVERLAPPED, FALSE);
	HWND hwnd = CreateWindowA("Diesel11", "Hello", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hThisInst, NULL);
	if (!hwnd) {
		return -1;
	}
	// http://gafferongames.com/game-physics/fix-your-timestep/
	float dt = 1.0f / 60.0f;
	DWORD _lastTime = GetTickCount();
	DWORD delta = 0;
	float accu = 0.0f;
	int frames = 0;
	int fps = 0;
	app->prepare(hThisInst, hwnd);
	//Demo demo;
	//demo.initialize();
	app->initialize();
	ShowWindow(hwnd, SW_SHOW);
	MSG msg = { 0 };
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		DWORD now = GetTickCount();
		delta += (now - _lastTime);
		float elapsed = (float)((now - _lastTime) * 0.001);

		_lastTime = now;
		accu += elapsed;
		if (delta > 1000) {
			delta = 0;
			fps = frames;
			frames = 0;
			LOG << "FPS:" << fps;
		}
		while (accu >= dt) {
			app->update(dt);
			accu -= dt;
		}
		
		graphics::beginRendering();
		app->render();
		graphics::endRendering();
		++frames;
	}
	delete app;
	graphics::shutdown();
	return 0;
}