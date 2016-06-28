#pragma once
#include <Windows.h>
#include "..\Common.h"
#include "..\utils\StringUtils.h"
#include "..\dialogs\GUIDialog.h"
#include "..\resources\ResourceContainer.h"

namespace ds {

// --------------------------------------------
// GameState
// --------------------------------------------
class GameState {

public:
	GameState(const char* name) : _name(name) , _initialized(false) {
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
	virtual int onGUIButton(int button) {
		return button;
	}
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
	bool isInitialized() const {
		return _initialized;
	}
	void endInitialisation() {
		_initialized = true;
	}
private:
	bool _initialized;
	GameState(const GameState& other) {}
	const char* _name;
	IdString _hash;
};

// --------------------------------------------
// Basic menu game state
// --------------------------------------------
class BasicMenuGameState : public GameState {

public:
	BasicMenuGameState(const char* name,const char* dialogName) : GameState(name) {
		_dialog = res::getGUIDialog(dialogName);
	}
	~BasicMenuGameState() {}
	int onButtonUp(int button, int x, int y) {
		//int ret = _dialog->onButton(button, x, graphics::getScreenHeight() - y, true);
		int ret = _dialog->onButton(button, x, y, true);
		if (ret != -1) {
			int tmp = onGUIButton(ret);
			if (tmp != -1) {
				return tmp;
			}
			return ret;
		}
		return 0;
	}
	void render() {
		_dialog->render();
	}
protected:
	GUIDialog* _dialog;
	const char* _dialogName;
};

}