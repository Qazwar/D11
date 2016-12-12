#include "GameStateMachine.h"
#include "core\log\Log.h"
#include "core\profiler\Profiler.h"
#include <core\base\Assert.h>

namespace ds {

	GameStateMachine::GameStateMachine() {
		_activeState = 0;
		_activeModalState = 0;
		_currentIndex = -1;
		_dialogPos = v2(10, graphics::getScreenHeight() - 10.0f);
		_dialogState = 1;
		_offset = 0;
	}

	GameStateMachine::~GameStateMachine() {
		_model.clear();
		_gameStates.destroy_all();
	}

	void GameStateMachine::add(GameState* gameState) {
		_gameStates.push_back(gameState);
		_model.add(gameState->getName(), gameState);
	}

	void GameStateMachine::initializeStates() {
		for (size_t i = 0; i < _gameStates.size(); ++i) {
			_gameStates[i]->init();
		}
	}
	void GameStateMachine::activate(const char* name) {
		int idx = find(name);
		XASSERT(idx != -1,"Cannot find matching gamestate for %s",name);
		if (idx != -1) {
			switchState(idx);
		}
	}

	void GameStateMachine::deactivate(const char* name) {
		int idx = find(name);
		XASSERT(idx != -1, "Cannot find matching gamestate for %s", name);
		if (idx != -1) {
			_gameStates[idx]->deactivate();
			_activeState = 0;
		}
	}

	bool GameStateMachine::contains(const char* name) const {
		return find(name) != -1;
	}

	void GameStateMachine::update(float dt) {
		ZoneTracker z("GameStateMachine:update");
		if (_activeState != 0) {
			int transition = _activeState->update(dt);
			handleStateTransition(transition);
		}
	}

	void GameStateMachine::render() {
		ZoneTracker z("GameStateMachine:render");
		if (_activeState != 0) {
			_activeState->render();
			if (gui::isInitialized()) {
				_activeState->renderGUI();
			}
		}
		if (_activeModalState != 0) {
			_activeModalState->render();
			if (gui::isInitialized()) {
				_activeModalState->renderGUI();
			}
		}
	}

	void GameStateMachine::onButtonDown(int button, int x, int y) {
		if (_activeModalState != 0) {
			int transition = _activeModalState->onButtonDown(button, x, y);
			handleStateTransition(transition);
		}
		else {
			if (_activeState != 0) {
				int transition = _activeState->onButtonDown(button, x, y);
				handleStateTransition(transition);
			}
		}
	}

	void GameStateMachine::onKeyDown(WPARAM virtualKey) {
		if (_activeModalState != 0) {
			int transition = _activeModalState->onKeyDown(virtualKey);
			handleStateTransition(transition);
		}
		else {
			if (_activeState != 0) {
				int transition = _activeState->onKeyDown(virtualKey);
				handleStateTransition(transition);
			}
		}
	}

	void GameStateMachine::onKeyUp(WPARAM virtualKey) {
		if (_activeModalState != 0) {
			int transition = _activeModalState->onKeyUp(virtualKey);
			handleStateTransition(transition);
		}
		else {
			if (_activeState != 0) {
				int transition = _activeState->onKeyUp(virtualKey);
				handleStateTransition(transition);
			}
		}
	}

	void GameStateMachine::onButtonUp(int button, int x, int y) {
		if (_activeModalState != 0) {
			int transition = _activeModalState->onButtonUp(button, x, y);
			handleStateTransition(transition);
		}
		else {
			if (_activeState != 0) {
				int transition = _activeState->onButtonUp(button, x, y);
				handleStateTransition(transition);
			}
		}
	}

	void GameStateMachine::onChar(int ascii) {
		if (_activeModalState != 0) {
			int transition = _activeModalState->onChar(ascii);
			handleStateTransition(transition);
		}
		else {
			if (_activeState != 0) {
				int transition = _activeState->onChar(ascii);
				handleStateTransition(transition);
			}
		}
	}

	void GameStateMachine::connect(const char* firstStateName, int outcome, const char* secondStateName) {
		XASSERT(outcome != 0, "You need to define an outcome");
		FSMConnection connection;
		connection.firstStateIndex = find(firstStateName);
		XASSERT(connection.firstStateIndex != -1,"Cannot find matching first state");
		connection.secondStateIndex = find(secondStateName);
		XASSERT(connection.secondStateIndex != -1,"Cannot find matching second state");		
		connection.outcome = outcome;
		_connections.push_back(connection);
	}

	void GameStateMachine::handleStateTransition(int outcome) {
		if (outcome != 0) {
			for (size_t i = 0; i < _connections.size(); ++i) {
				const FSMConnection& con = _connections[i];
				if (con.firstStateIndex == _currentIndex && con.outcome == outcome) {
					switchState(con.secondStateIndex);
					break;
				}
			}
		}
	}

	int GameStateMachine::find(const char* name) const {
		StaticHash hash(name);
		for (size_t i = 0; i < _gameStates.size(); ++i) {
			if (_gameStates[i]->getHash() == hash) {
				return i;
			}
		}
		return -1;
	}

	void GameStateMachine::switchState(int newIndex) {
		if (_gameStates[newIndex]->isModal()) {
			_activeModalState = _gameStates[newIndex];
			_activeModalState->activate();
		}
		else {
			if (_activeState != 0) {
				LOG << "deactivating " << _activeState->getName();
				_activeState->deactivate();
			}
			_activeState = _gameStates[newIndex];
			if (!_activeState->isInitialized()) {
				_activeState->init();
				_activeState->endInitialisation();
			}
			LOG << "activating " << _activeState->getName();
			_activeState->activate();
			_currentIndex = newIndex;
		}
	}
	
	void GameStateMachine::showDialog() {
		ZoneTracker z("GameStateMachine::showDialog");
		gui::start(100,&_dialogPos);
		if (gui::begin("GameStates", &_dialogState)) {
			gui::DropDownBox(&_model, &_offset);
			if (gui::Button("Activate")) {
				if (_model.hasSelection()) {
					switchState(_model.getSelection());
				}
			}
		}
		gui::end();
	}
	
}
