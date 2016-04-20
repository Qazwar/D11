#include <Windows.h>
#include "BaseApp.h"

extern ds::BaseApp* app;

#define GETX(l) (int(l & 0xFFFF))
#define GETY(l) (int(l) >> 16)

LRESULT CALLBACK WinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_PAINT:
			if (app->isLoading()) {
				HDC hDC = GetDC(hwnd);
				TCHAR strMsg[MAX_PATH];
				wsprintf(strMsg, TEXT("Loading data - Please wait"));
				RECT rct;
				GetClientRect(hwnd, &rct);
				SetTextColor(hDC, RGB(255, 255, 255));
				SetBkMode(hDC, TRANSPARENT);
				DrawText(hDC, strMsg, -1, &rct, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				ReleaseDC(hwnd, hDC);
			}
			break;
		case WM_CHAR: {
			char ascii = wParam;
			unsigned int keyState = lParam;
			app->sendOnChar(ascii, keyState);
		}
			return 0;
		case WM_KEYDOWN:
			//app->sendKeyDown(wParam);
			return 0;
		case WM_KEYUP:
			//app->sendKeyUp(wParam);
			return 0;
		case WM_LBUTTONDOWN:
			app->sendButton(0, GETX(lParam), GETY(lParam), true);
			break;
		case WM_LBUTTONUP:
			app->sendButton(0, GETX(lParam), GETY(lParam), false);
			break;
		case WM_RBUTTONDOWN:
			app->sendButton(1, GETX(lParam), GETY(lParam), true);
			break;
		case WM_RBUTTONUP:
			app->sendButton(1, GETX(lParam), GETY(lParam), false);
			break;
		case WM_DESTROY :
			PostQuitMessage(0);
			break;
		}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hThisInst, HINSTANCE hLastInst, LPSTR lpszCmdLine, int nCmdShow) {
#ifdef _DEBUG
	int flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG); // Get current flag
	flag |= _CRTDBG_LEAK_CHECK_DF; // Turn on leak-checking bit
	flag |= _CRTDBG_CHECK_ALWAYS_DF; // Turn on CrtCheckMemory
	//flag |= _CRTDBG_DELAY_FREE_MEM_DF;
	_CrtSetDbgFlag(flag); // Set flag to the new value
#endif
	WNDCLASSEX wndClass = { 0 };
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WinProc;
	wndClass.hInstance = hThisInst;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = "D11";

	if (!RegisterClassEx(&wndClass)) {
		return -1;
	}
	app->setInstance(hThisInst);
	app->createWindow();
	app->prepare();
	app->initialize();
	MSG msg = { 0 };
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		app->buildFrame();
	}
	delete app;	
	return 0;
}