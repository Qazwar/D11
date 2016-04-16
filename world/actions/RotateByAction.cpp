#include "RotateByAction.h"
#include "..\..\utils\Log.h"
#include "..\..\math\GameMath.h"

namespace ds {
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	RotateByAction::RotateByAction() : AbstractAction("rotate_by") {
		int sizes[] = { sizeof(SID), sizeof(float), sizeof(float), sizeof(float), sizeof(float), sizeof(float), sizeof(tweening::TweeningType), sizeof(int) };
		_buffer.init(sizes, 8);
	}

	void RotateByAction::allocate(int sz) {
		if (_buffer.resize(sz)) {
			_ids = (SID*)_buffer.get_ptr(0);
			_startAngles = (float*)_buffer.get_ptr(1);
			_endAngles = (float*)_buffer.get_ptr(2);
			_steps = (float*)_buffer.get_ptr(3);
			_timers = (float*)_buffer.get_ptr(4);
			_ttl = (float*)_buffer.get_ptr(5);
			_tweeningTypes = (tweening::TweeningType*)_buffer.get_ptr(6);
			_modes = (int*)_buffer.get_ptr(7);
		}
	}
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void RotateByAction::attach(SID id,float startAngle,float endAngle,float ttl,int mode,const tweening::TweeningType& tweeningType) {
		int idx = create(id);
		_ids[idx] = id;
		_startAngles[idx] = startAngle;
		_endAngles[idx] = endAngle;
		_steps[idx] = endAngle - startAngle;
		_timers[idx] = 0.0f;
		_ttl[idx] = ttl;
		_tweeningTypes[idx] = tweeningType;
		_modes[idx] = mode;
		if ( mode > 0 ) {
			--_modes[idx];
		}
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void RotateByAction::update(SpriteArray& array,float dt,ActionEventBuffer& buffer) {	
		if (_buffer.size > 0) {
			// move
			for (int i = 0; i < _buffer.size; ++i) {
				array.rotate(_ids[i], tweening::interpolate(_tweeningTypes[i], _startAngles[i], _endAngles[i], _timers[i], _ttl[i]));
				_timers[i] += dt;
				if ( _timers[i] >= _ttl[i] ) {
					if ( _modes[i] < 0 ) {
						_timers[i] = 0.0f;
					}
					else if ( _modes[i] == 0 ) {
						array.rotate(_ids[i], _endAngles[i]);
						buffer.add(_ids[i], AT_ROTATE_BY, array.getType(_ids[i]));
						removeByIndex(i);
					}
					else {
						float current = array.getRotation(_ids[i]);
						_startAngles[i] = current;
						_endAngles[i] = current + _steps[i];
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
	void RotateByAction::debug() {
		for (int i = 0; i < _buffer.size; ++i) {
			LOG << i << " : id: " << _ids[i] << " start: " << _startAngles[i] << " end: " << _endAngles[i] << " ttl: " << _ttl[i] << " timer: " << _timers[i];
		}
	}

	void RotateByAction::debug(SID sid) {
		int i = find(sid);
		if (i != -1) {
			LOG << "> rotate by : id: " << _ids[i] << " start: " << _startAngles[i] << " end: " << _endAngles[i] << " ttl: " << _ttl[i] << " timer: " << _timers[i];
		}
	}

}