#include "RotateAction.h"
#include "..\..\utils\Log.h"
#include "..\..\math\GameMath.h"

namespace ds {
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	RotateAction::RotateAction() : AbstractAction("rotate") {
		int sizes[] = { sizeof(SID), sizeof(float), sizeof(float), sizeof(float), sizeof(int) };
		_buffer.init(sizes, 5);
	}

	void RotateAction::allocate(int sz) {
		if (_buffer.resize(sz)) {
			_ids = (SID*)_buffer.get_ptr(0);
			_angleVelocities  = (float*)_buffer.get_ptr(1);
			_timers = (float*)_buffer.get_ptr(2);
			_ttl = (float*)_buffer.get_ptr(3);
			_modes = (int*)_buffer.get_ptr(4);
		}
	}
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void RotateAction::attach(SID id,float angleVelocity,float ttl,int mode) {
		int idx = create(id);
		_ids[idx] = id;
		_angleVelocities[idx] = DEGTORAD(angleVelocity);
		_timers[idx] = 0.0f;
		_ttl[idx] = ttl;
		_modes[idx] = mode;
		if ( mode > 0 ) {
			--_modes[idx];
		}
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void RotateAction::update(SpriteArray& array,float dt,ActionEventBuffer& buffer) {	
		if (_buffer.size > 0) {
			// move
			for (int i = 0; i < _buffer.size; ++i) {
				float angle = array.getRotation(_ids[i]);
				angle += _angleVelocities[i] * dt;
				array.rotate(_ids[i], angle);
				_timers[i] += dt;
				if ( _timers[i] >= _ttl[i] ) {
					if ( _modes[i] < 0 ) {
						_timers[i] = 0.0f;
					}
					else if ( _modes[i] == 0 ) {
						removeByIndex(i);
					}
					else {
						float current = array.getRotation(_ids[i]);
						--_modes[i];
						_timers[i] = 0.0f;

					}
				}
			}
		}
	}
	
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void RotateAction::debug() {
		for (int i = 0; i < _buffer.size; ++i) {
			LOG << i << " : id: " << _ids[i] << " angle velocity: " << _angleVelocities[i] << " ttl: " << _ttl[i] << " timer: " << _timers[i];
		}
	}

	void RotateAction::debug(SID sid) {
		int i = find(sid);
		if (i != -1) {
			LOG << "RotateAction - id: " << _ids[i] << " angle velocity: " << _angleVelocities[i] << " ttl: " << _ttl[i] << " timer: " << _timers[i];
		}
	}

}