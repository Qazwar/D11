#pragma once
#include <Windows.h>

class BaseApp {

public:
	BaseApp();
	~BaseApp();
	bool prepare(HINSTANCE hInstance, HWND hwnd);
	virtual bool initialize() = 0;
	virtual void render() = 0;	
	virtual void update(float dt) = 0;
};

