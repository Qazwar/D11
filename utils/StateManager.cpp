#include "StateManager.h"
#include "Log.h"
#include "..\imgui\IMGUI.h"
#include "..\utils\Assert.h"

namespace ds {

	// -------------------------------------------------------
	// State Manager
	// -------------------------------------------------------	
	StateManager::~StateManager() {
		// remove all states
		std::map<int, State*>::iterator it = _states.begin();
		while (it != _states.end()) {
			delete it->second;
			++it;
		}

	}

	// -------------------------------------------------------
	// add transition
	// -------------------------------------------------------
	void StateManager::addTransition(int from, int outcome, int to, float ttl) {
		// FIXME: assert that from and to are valid
		XASSERT(from != to, "You cannot add a transition pointing to itself");
		XASSERT(_states.find(from) != _states.end(), "The from %d paramter is an unknown state",from);
		XASSERT(_states.find(to) != _states.end(), "The to %d paramter is an unknown state",to);
		StateTransition t;
		t.from = from;
		t.to = to;
		t.outcome = outcome;
		t.ttl = ttl;
		_transitions.push_back(t);
	}

	// -------------------------------------------------------
	// find transition
	// -------------------------------------------------------
	int StateManager::findTransition(int from, int outcome) {
		for (int i = 0; i < _transitions.size(); ++i) {
			const StateTransition& t = _transitions[i];
			if (t.from == from && t.outcome == outcome) {
				return i;
			}
		}
		return -1;
	}

	// -------------------------------------------------------
	// activate
	// -------------------------------------------------------
	void StateManager::activate(int mode) {
		_next = mode;
	}

	int StateManager::deactivateState(int index, bool checkOutcome) {
		if (index != -1) {
			//LOG << "deactivating: " << _states[index]->getName();
			int outcome = _states[index]->deactivate();
			//LOG << "outcome: " << outcome;
			if (outcome != 0 && checkOutcome ) {
				return findTransition(index, outcome);
			}
		}
		return -1;
	}

	// -------------------------------------------------------
	// switch state
	// -------------------------------------------------------
	bool StateManager::switchState() { 
		if (_next != -1 && _next != _current) {
			//LOG << "switching from " << _current << " to " << _next;
			int outcome = 0;
			if (_next != -1) {
				if (_current != -1) {
					deactivateState(_current, false);
				}
				_current = _next;
				_states[_current]->resetTimer();
				//LOG << "activating: " << _states[_current]->getName();
				outcome = _states[_current]->activate();
				//LOG << "outcome: " << outcome;
				int idx = findTransition(_current, outcome);
				if (outcome != 0 && idx != -1) {
					activate(_transitions[idx].to);
				}
				else {
					_next = -1;
				}
				if (_states[_current]->getBehavior() == SB_TRANSIENT) {
					_ttl = _transitions[idx].ttl;

				}
				else {
					_ttl = 0.0f;
				}
				//LOG << "TTL: " << _ttl;				
				return true;
			}
		}
		return false;
	}

	// -------------------------------------------------------
	// tick
	// -------------------------------------------------------
	void StateManager::tick(float dt) {
		if (!switchState()) {
			if (_current != -1) {			
				int outcome = _states[_current]->update(dt);
				if (_states[_current]->getBehavior() == SB_TRANSIENT) {
					_states[_current]->tickTimer(dt);
					if (_states[_current]->getTimer() >= _ttl) {
						//LOG << "timer expired";
						int ret = deactivateState(_current, true);
						int idx = findTransition(_current, outcome);
						if (idx != -1) {
							StateTransition t = _transitions[idx];
							activate(t.to);
						}
						else if (ret != -1) {
							StateTransition t = _transitions[ret];
							activate(t.to);
						}
						else {
							//LOG << "no next state";
							_current = -1;
						}
					}
				}
				else if (_states[_current]->getBehavior() == SB_ONETIME) {
					int ret = deactivateState(_current, false);
					int idx = findTransition(_current, outcome);
					if (idx != -1) {
						activate(_transitions[idx].to);
					}
					if (ret != -1) {
						activate(_transitions[ret].to);
					}
				}
				if (outcome != 0) {
					int idx = findTransition(_current, outcome);
					if (idx != -1) {
						activate(_transitions[idx].to);
					}
					else {
						_current = -1;
					}
				}
			}
		}
	}

	// -------------------------------------------------------
	// stop
	// -------------------------------------------------------
	void StateManager::stop() {
		if (_current != -1) {
			_states[_current]->deactivate();
		}
		_current = -1;
	}

	const char* translateBehavior(StateBehavior behavior) {
		switch (behavior) {
			case SB_TRANSIENT: return "Transient"; break;
			case SB_PERMANENT: return "Permanent"; break;
			case SB_ONETIME: return "OneTime"; break;
			default: return "Unknown"; break;
		}
	}

	// -------------------------------------------------------
	// showDialog
	// -------------------------------------------------------
	void StateManager::showDialog() {
		gui::start(1, &_dialogPos);
		gui::begin("State", &_dialogState);
		if (_current != -1) {
			gui::Label("Active", _states[_current]->getName());
			gui::Value("TTL", _ttl);
			gui::Value("Timer", _states[_current]->getTimer());
			gui::Label("Behavior", translateBehavior(_states[_current]->getBehavior()));
		}
		else {
			gui::Label("No active state");
		}		
		gui::end();
	}

}