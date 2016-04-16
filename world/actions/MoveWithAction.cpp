#include "MoveWithAction.h"
#include "..\..\utils\Log.h"
#include "..\..\math\GameMath.h"

namespace ds {
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	MoveWithAction::MoveWithAction() : AbstractAction("move_with") {
		int sizes[] = { sizeof(SID), sizeof(MoveFunc), sizeof(float), sizeof(float) , sizeof(int)};
		_buffer.init(sizes, 5);
	}

	void MoveWithAction::allocate(int sz) {
		if (_buffer.resize(sz)) {
			_ids = (SID*)_buffer.get_ptr(0);
			_functions = (MoveFunc*)_buffer.get_ptr(1);
			_timers = (float*)_buffer.get_ptr(2);
			_ttl = (float*)_buffer.get_ptr(3);
			_modes = (int*)_buffer.get_ptr(4);
		}
	}
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void MoveWithAction::attach(SID id,const MoveFunc& function,float ttl,int mode) {
		int idx = create(id);
		_ids[idx] = id;
		_timers[idx] = 0.0f;
		_functions[idx] = function;
		_ttl[idx] = ttl;
		_modes[idx] = mode;
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void MoveWithAction::update(SpriteArray& array,float dt,ActionEventBuffer& buffer) {	
		if (_buffer.size > 0) {
			// move
			float angle = 0.0f;
			Vector2f p;
			for (int i = 0; i < _buffer.size; ++i) {
				float norm = _timers[i] / _ttl[i];				
				_functions[i](p,&angle,norm);
				_timers[i] += dt;
				array.setPosition(_ids[i], p);
				array.rotate(_ids[i], angle);
				if (_timers[i] >= _ttl[i]) {
					if (_modes[i] < 0) {
						_timers[i] = 0.0f;
					}
					else if (_modes[i] == 0) {
						_functions[i](p, &angle, 1.0f);
						array.setPosition(_ids[i], p);
						buffer.add(_ids[i], AT_MOVE_WITH, array.getType(_ids[i]));
						removeByIndex(i);
					}
					else {
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
	void MoveWithAction::debug() {
		LOG << "------- MoveWithAction -------";
		for (int i = 0; i < _buffer.size; ++i) {
			LOG << i << " id: " << _ids[i] << " timer: " << _timers[i];
		}		
	}

}