#include "WaitAction.h"
#include "..\..\math\GameMath.h"
#include "..\..\utils\Log.h"

namespace ds {
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	WaitAction::WaitAction() : AbstractAction("wait") {
		int sizes[] = { sizeof(SID), sizeof(float), sizeof(float) };
		_buffer.init(sizes, 3);
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	WaitAction::~WaitAction() {
	}

	void WaitAction::allocate(int sz) {
		if (_buffer.resize(sz)) {
			_ids = (SID*)_buffer.get_ptr(0);
			_timers = (float*)_buffer.get_ptr(1);
			_ttl = (float*)_buffer.get_ptr(2);
		}
	}
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void WaitAction::attach(SID id,float ttl) {
		int idx = create(id);
		_ids[idx] = id;
		_timers[idx] = 0.0f;
		_ttl[idx] = ttl;
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void WaitAction::update(SpriteArray& array,float dt,ActionEventBuffer& buffer) {
		if ( _buffer.size > 0 ) {				
			// move
			for (int i = 0; i < _buffer.size; ++i) {
				_timers[i] += dt;
				if ( _timers[i] >= _ttl[i] ) {
					buffer.add(_ids[i],AT_WAIT,array.getType(_ids[i]));
					removeByIndex(i);	
				}
			}
		}
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void WaitAction::debug() {
		if (_buffer.size > 0) {
			LOG << "---------- WaitAction ---------- ";
		}
		for (int i = 0; i < _buffer.size; ++i) {
			LOG << i << " : id: " << _ids[i] << " ttl: " << _ttl[i] << " timer: " << _timers[i];
		}
	}

	void WaitAction::debug(SID sid) {
		int i = find(sid);
		if (i != -1) {
			LOG << "WaitAction - id: " << _ids[i] << " ttl: " << _ttl[i] << " timer: " << _timers[i];
		}
	}

}