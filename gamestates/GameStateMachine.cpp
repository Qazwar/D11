#include "GameStateMachine.h"
#include "core\log\Log.h"
#include "core\profiler\Profiler.h"
#include <core\base\Assert.h>

namespace ds {

	GameStateMachine::GameStateMachine() {
		_dialogPos = v2(10, graphics::getScreenHeight() - 10.0f);
		_dialogState = 1;
		_offset = 0;
	}

	GameStateMachine::~GameStateMachine() {
		_model.clear();
		_gameStates.destroy_all();
	}

	// -------------------------------------------------------
	// add game state
	// -------------------------------------------------------
	void GameStateMachine::add(GameState* gameState) {
		_gameStates.push_back(gameState);
		_model.add(gameState->getName(), gameState);
	}

	// -------------------------------------------------------
	// initialize states
	// -------------------------------------------------------
	void GameStateMachine::initializeStates() {
		for (size_t i = 0; i < _gameStates.size(); ++i) {
			_gameStates[i]->init();
		}
	}
	
	// -------------------------------------------------------
	// contains
	// -------------------------------------------------------
	bool GameStateMachine::contains(const char* name) const {
		return find(name) != -1;
	}

	// -------------------------------------------------------
	// update
	// -------------------------------------------------------
	void GameStateMachine::update(float dt) {
		ZoneTracker z("GameStateMachine:update");
		for (uint32_t i = 0; i < _stack.size(); ++i) {
		//if (_activeState != 0) {
			int transition = _stack.at(i)->update(dt);
			handleStateTransition(transition);
		}
	}

	// -------------------------------------------------------
	// render
	// -------------------------------------------------------
	void GameStateMachine::render() {
		ZoneTracker z("GameStateMachine:render");
		if (!_stack.empty()) {
			for (int i = _stack.size() - 1; i >= 0; --i) {
				GameState* state = _stack.at(i);
				state->render();
				if (gui::isInitialized()) {
					state->renderGUI();
				}
			}
		}
	}

	// -------------------------------------------------------
	// on button down
	// -------------------------------------------------------
	void GameStateMachine::onButtonDown(int button, int x, int y) {
		if (!_stack.empty()) {
			GameState* activeState = _stack.top();
			int transition = activeState->onButtonDown(button, x, y);
			handleStateTransition(transition);
		}
	}

	// -------------------------------------------------------
	// on button up
	// -------------------------------------------------------
	void GameStateMachine::onButtonUp(int button, int x, int y) {
		if (!_stack.empty()) {
			GameState* activeState = _stack.top();
			int transition = activeState->onButtonUp(button, x, y);
			handleStateTransition(transition);
		}
	}

	// -------------------------------------------------------
	// on char
	// -------------------------------------------------------
	void GameStateMachine::onChar(int ascii) {
		if (!_stack.empty()) {
			GameState* activeState = _stack.top();
			int transition = activeState->onChar(ascii);
			handleStateTransition(transition);
		}
	}

	// -------------------------------------------------------
	// push state
	// -------------------------------------------------------
	void GameStateMachine::push(const char* name) {
		int idx = find(name);
		XASSERT(idx != -1, "Cannot find matching gamestate for %s", name);
		if (idx != -1) {
			switchState(idx);
		}
	}

	// -------------------------------------------------------
	// pop state
	// -------------------------------------------------------
	void GameStateMachine::pop() {
		if (!_stack.empty()) {
			GameState* active = _stack.top();
			active->deactivate();
			_stack.pop();
		}
	}

	// -------------------------------------------------------
	// connect two game states
	// -------------------------------------------------------
	void GameStateMachine::connect(const char* firstStateName, int outcome, const char* secondStateName) {
		XASSERT(outcome != 0, "You need to define an outcome");
		GameStateConnection connection;
		connection.firstState = SID(firstStateName);
		XASSERT(find(connection.firstState) != -1,"Cannot find matching first state");
		connection.secondState = SID(secondStateName);
		XASSERT(find(connection.secondState) != -1,"Cannot find matching second state");		
		connection.outcome = outcome;
		_connections.push_back(connection);
	}

	// -------------------------------------------------------
	// handle transition
	// -------------------------------------------------------
	void GameStateMachine::handleStateTransition(int outcome) {
		if (outcome != 0) {
			GameState* current = _stack.top();
			StaticHash currentHash = current->getHash();
			LOG << "deactivating: " << current->getName();
			current->deactivate();
			_stack.pop();
			for (size_t i = 0; i < _connections.size(); ++i) {
				const GameStateConnection& con = _connections[i];
				if (con.firstState == currentHash && con.outcome == outcome) {
					int idx = find(con.secondState);
					if (idx != -1) {
						switchState(idx);
						break;
					}
				}
			}
		}
	}

	// -------------------------------------------------------
	// find by name
	// -------------------------------------------------------
	int GameStateMachine::find(const char* name) const {
		StaticHash hash(name);
		return find(hash);
	}

	// -------------------------------------------------------
	// find by hash
	// -------------------------------------------------------
	int GameStateMachine::find(const StaticHash& hash) const {
		for (size_t i = 0; i < _gameStates.size(); ++i) {
			if (_gameStates[i]->getHash() == hash) {
				return i;
			}
		}
		return -1;
	}

	// -------------------------------------------------------
	// switch state
	// -------------------------------------------------------
	void GameStateMachine::switchState(int newIndex) {
		GameState* next = _gameStates[newIndex];
		if (!next->isInitialized()) {
			next->init();
			next->endInitialisation();
		}
		LOG << "activating " << next->getName();
		next->activate();
		_stack.push(next);
	}
	
	// -------------------------------------------------------
	// show dialog
	// -------------------------------------------------------
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
