#pragma once
#include <Windows.h>
#include "core\Common.h"
#include "core\string\StringUtils.h"
#include "..\dialogs\GUIDialog.h"
#include "..\resources\ResourceContainer.h"
#include "core\base\EventStream.h"

namespace ds {

// --------------------------------------------
// GameState
// --------------------------------------------
class GameState {

public:
	GameState(const char* name) : _name(name) , _initialized(false) , _hash(name) {
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
	const StaticHash& getHash() const {
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
	void operator=(const GameState& other) {}
	const char* _name;
	StaticHash _hash;
};

// --------------------------------------------
// Basic menu game state
// --------------------------------------------
class BasicMenuGameState : public GameState , public GUIHoverCallback {

public:
	BasicMenuGameState(const char* name, const char* dialogName) : GameState(name), _dialogName(dialogName) {
		_dialog = res::getGUIDialog(dialogName);
	}
	~BasicMenuGameState() {}
	virtual int onButtonUp(int button, int x, int y) {
		int ret = _dialog->onButton(button, x, y, true);
		if (ret != -1) {
			if (ret == 666) {
				events::send(InternalEvents::ENGINE_SHUTDOWN);
				return 0;
			}
			int tmp = onGUIButton(ret);
			if (tmp != -1) {
				return tmp;
			}
			return ret;
		}
		return 0;
	}
	virtual void activate() {
		_dialog->activate();
	}
	virtual void deactivate() {
		_dialog->deactivate();
	}
	virtual void render() {
		graphics::turnOffZBuffer();
		_dialog->render();
		graphics::turnOnZBuffer();
	}
	virtual int onChar(int ascii) {
#ifdef DEBUG
		if (ascii == 'r') {
			res::reloadDialog(_dialogName);
		}
#endif
		return 0;
	}
	virtual void entering(ID id) {}
	virtual void leaving(ID id) {}
protected:
	GUIDialog* _dialog;
	const char* _dialogName;
};

}