#pragma once
#include "GameState.h"
#include "core\log\Log.h"
#include "core\lib\collection_types.h"

namespace ds {

class GameStateMachine {

struct FSMConnection {
	int firstStateIndex;
	int secondStateIndex;
	int outcome;
};

typedef Array<GameState*> GameStates;
typedef Array<FSMConnection> Connections;

public:
	GameStateMachine();
	~GameStateMachine();
	template<class T>
	void add(const char* name) {
		T* t = new T(name);
		t->init();
		_gameStates.push_back(t);
		_model.add(name, t);
	}
	void add(GameState* gameState);
	void initializeStates();
	void activate(const char* name);
	void deactivate(const char* name);
	bool contains(const char* name) const;
	void update(float dt);
	void render();
	void onButtonDown(int button, int x, int y);
	void onKeyDown(WPARAM virtualKey);
	void onKeyUp(WPARAM virtualKey);
	void onButtonUp(int button, int x, int y);
	void onChar(int ascii);
	void connect(const char* firstStateName, int outcome, const char* secondStateName);
	void showDialog();
	//void processEvents(const EventStream& events);
	void init() {}
	const char* getShortcut() const {
		return "GSM";
	}
private:
	void handleStateTransition(int outcome);
	int find(const char* name) const;
	void switchState(int newIndex);
	int _currentIndex;
	GameState* _activeState;
	GameStates _gameStates;
	Connections _connections;
	//gui::ComponentModel<GameState*> _model;
	//v2 _dialogPos;
	//int _dialogState;
	//int _offset;
};

}
