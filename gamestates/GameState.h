#pragma once
#include <Windows.h>
#include "..\Common.h"
#include "..\utils\StringUtils.h"

namespace ds {

// --------------------------------------------
// GameState
// --------------------------------------------
class GameState {

public:
	GameState(const char* name) : _name(name) {
		_hash = string::murmur_hash(name);
	}
	virtual ~GameState(void) {}
	virtual void init() {}
	virtual int update(float dt) {
		return 0;
	}
	virtual void render() {}
	// callbacks
	virtual void activate() {}
	virtual void deactivate() {}
	virtual int onButtonDown(int button,int x,int y) {
		return 0;
	}
	virtual int onButtonUp(int button, int x, int y) {
		return 0;
	}
	virtual int onChar(int ascii) {
		return 0;
	}
	//virtual int onGUIButton(ds::DialogID dlgID, int button) {
		//return button;
	//}
	virtual int onKeyDown(WPARAM virtualKey) {
		return 0;
	}
	virtual int onKeyUp(WPARAM virtualKey) {
		return 0;
	}
	//virtual int processEvents(const EventStream& events) {
		//return 0;
	//}
	IdString getHash() const {
		return _hash;
	}
	const char* getName() const {
		return _name;
	}
private:
	GameState(const GameState& other) {}
	const char* _name;
	IdString _hash;
};

// --------------------------------------------
// Basic menu game state
// --------------------------------------------
/*
class BasicMenuGameState : public GameState {

public:
	BasicMenuGameState(const char* name, const char* dialogName, ds::DialogManager* gui) : GameState(name) {
		_dialogName = dialogName;
		_gui = gui;
	}
	~BasicMenuGameState() {}
	void activate() {
		_gui->activate(_dialogName);
	}
	void deactivate() {
		_gui->deactivate(_dialogName);
	}
	int onGUIButton(ds::DialogID dlgID, int button) {
		return button;
	}
private:
	const char* _dialogName;
	ds::DialogManager* _gui;
};
*/
}