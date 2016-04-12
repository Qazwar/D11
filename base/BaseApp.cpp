#include "BaseApp.h"
#include "..\graphics.h"

BaseApp::BaseApp()
{
}


BaseApp::~BaseApp()
{
}


bool BaseApp::prepare(HINSTANCE hInstance, HWND hwnd) {
	return graphics::initialize(hInstance, hwnd);
}