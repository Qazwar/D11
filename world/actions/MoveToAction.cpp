#include "MoveToAction.h"
#include "..\..\utils\Log.h"
#include "..\..\math\GameMath.h"

namespace ds {
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	MoveToAction::MoveToAction() : AbstractAction("move_to") {
		int sizes[] = { sizeof(SID), sizeof(v2), sizeof(v2), sizeof(float), sizeof(float), sizeof(tweening::TweeningType), sizeof(int) };
		_buffer.init(sizes, 7);
	}

	void MoveToAction::allocate(int sz) {
		if (_buffer.resize(sz)) {
			_ids = (SID*)_buffer.get_ptr(0);
			_startPositions = (v2*)_buffer.get_ptr(1);
			_endPositions = (v2*)_buffer.get_ptr(2);
			_timers = (float*)_buffer.get_ptr(3);
			_ttl = (float*)_buffer.get_ptr(4);
			_tweeningTypes = (tweening::TweeningType*)_buffer.get_ptr(5);
			_modes = (int*)_buffer.get_ptr(6);
		}
	}
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void MoveToAction::attach(SID id, SpriteArray& array,const Vector2f& startPos, const Vector2f& endPos, float ttl, int mode, const tweening::TweeningType& tweeningType) {
		int idx = create(id);
		_ids[idx] = id;
		_startPositions[idx] = startPos;
		_endPositions[idx] = endPos;
		_timers[idx] = 0.0f;
		_ttl[idx] = ttl;
		_tweeningTypes[idx] = tweeningType;
		_modes[idx] = mode;
		if ( mode > 0 ) {
			--_modes[idx];
		}
		array.setPosition(id, startPos);
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void MoveToAction::update(SpriteArray& array,float dt,ActionEventBuffer& buffer) {	
		if (_buffer.size > 0) {
			// move
			for (int i = 0; i < _buffer.size; ++i) {
				array.setPosition(_ids[i], tweening::interpolate(_tweeningTypes[i], _startPositions[i], _endPositions[i], _timers[i], _ttl[i]));
				_timers[i] += dt;
				if ( _timers[i] >= _ttl[i] ) {
					if ( _modes[i] < 0 ) {
						_timers[i] = 0.0f;
					}
					else if ( _modes[i] == 0 ) {
						array.setPosition(_ids[i], _endPositions[i]);
						buffer.add(_ids[i], AT_MOVE_TO, array.getType(_ids[i]));
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
	void MoveToAction::debug() {
		LOG << "------- MoveToAction -------";
		for (int i = 0; i < _buffer.size; ++i) {
			LOG << i << " id: " << _ids[i] << " start: " << DBG_V2(_startPositions[i]) << " end: " << DBG_V2(_endPositions[i]) << " ttl: " << _ttl[i] << " timer: " << _timers[i];
		}		
	}

	void MoveToAction::debug(SID sid) {
		int i = find(sid);
		if (i != -1) {
			LOG << "> move_to : id: " << _ids[i] << " start: " << DBG_V2(_startPositions[i]) << " end: " << DBG_V2(_endPositions[i]) << " ttl: " << _ttl[i] << " timer: " << _timers[i];
		}
	}

}