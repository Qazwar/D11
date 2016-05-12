#pragma once
#include "..\lib\collection_types.h"
#include <map>
#include "..\base\EventStream.h"

namespace ds {

	enum StateBehavior {
		SB_TRANSIENT,
		SB_PERMANENT,
		SB_ONETIME,
		SB_EOL
	};
	
	// -------------------------------------------------------
	// State context
	// -------------------------------------------------------
	struct StateContext {

	};

	
	// -------------------------------------------------------
	// Base State
	// -------------------------------------------------------
	class State {

	public:
		State() {}
		virtual ~State() {}
		void prepare(StateContext* context) {
			_ctx = context;
		}
		virtual void init() {

		}
		virtual int activate() {
			return 0;
		}
		virtual int deactivate() {
			return 0;
		}
		virtual int update(float dt) {
			return 0;
		}
		virtual int getMode() const = 0;
		virtual const char* getName() const = 0;
		void sendEvent(uint32_t type) {
			events::send(type);
		}
		void sendEvent(uint32_t type, void* data, int size) {
			events::send(type, data, size);
		}
		virtual StateBehavior getBehavior() const = 0;
		float getTimer() const {
			return _timer;
		}
		void resetTimer() {
			_timer = 0.0f;
		}
		void tickTimer(float dt) {
			_timer += dt;
		}
	protected:
		StateContext* _ctx;		
	private:
		float _timer;
	};

	// -------------------------------------------------------
	// State transition
	// -------------------------------------------------------
	struct StateTransition {
		int from;
		int to;
		int outcome;
		float ttl;
	};
	// -------------------------------------------------------
	// State manager
	// -------------------------------------------------------
	class StateManager {

	public:
		StateManager(StateContext* ctx) : _current(-1), _next(-1), _context(ctx), _dialogState(1), _dialogPos(10, 720) {}
		~StateManager();
		template<class T>
		State* add() {
			T* t = new T();
			t->prepare(_context);
			t->init();
			_states[t->getMode()] = t;
			return t;
		}
		void addTransition(int from, int outcome, int to, float ttl = 0.0f);
		void activate(int mode);
		void tick(float dt);
		void stop();
		int getCurrentMode() const {
			return _current;
		}
		void showDialog();
	private:
		bool switchState();
		int deactivateState(int index, bool checkOutcome = true);
		int findTransition(int from, int outcome);
		StateContext* _context;
		std::map<int, State*> _states;
		Array<StateTransition> _transitions;
		int _current;
		int _next;
		float _ttl;
		int _dialogState;
		v2 _dialogPos;
	};
}