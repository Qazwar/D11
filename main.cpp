#include <Windows.h>
#include "graphics.h"
#include "Demo.h"

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
	RECT rc = { 0, 0, 800, 600 };
	AdjustWindowRect(&rc, WS_OVERLAPPED, FALSE);
	HWND hwnd = CreateWindowA("Diesel11", "Hello", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hThisInst, NULL);
	if (!hwnd) {
		return -1;
	}
	graphics::initialize(hThisInst, hwnd);
	Demo demo;
	demo.initialize();
	ShowWindow(hwnd, SW_SHOW);
	MSG msg = { 0 };
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		graphics::beginRendering();
		demo.render();
		graphics::endRendering();
	}
	demo.shutdown();
	graphics::shutdown();
	return 0;
}