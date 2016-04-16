#include "ScalingAction.h"
#include "..\..\math\GameMath.h"
#include "..\..\utils\Log.h"

namespace ds {
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	ScalingAction::ScalingAction() : AbstractAction("scale") {
		int sizes[] = { sizeof(SID), sizeof(v2), sizeof(v2), sizeof(float), sizeof(float), sizeof(tweening::TweeningType), sizeof(int) };
		_buffer.init(sizes, 7);
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	ScalingAction::~ScalingAction() {}

	void ScalingAction::allocate(int sz) {
		if (_buffer.resize(sz)) {
			_ids = (SID*)_buffer.get_ptr(0);
			_startScale = (v2*)_buffer.get_ptr(1);
			_endScale = (v2*)_buffer.get_ptr(2);
			_timers = (float*)_buffer.get_ptr(3);
			_ttl = (float*)_buffer.get_ptr(4);
			_tweeningTypes = (tweening::TweeningType*)_buffer.get_ptr(5);
			_modes = (int*)_buffer.get_ptr(6);
		}
	}
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void ScalingAction::attach(SID id,const Vector2f& startScale,const Vector2f& endScale,float ttl,int mode,const tweening::TweeningType& tweeningType) {
		int idx = create(id);
		_ids[idx] = id;
		_startScale[idx] = startScale;
		_endScale[idx] = endScale;
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
	void ScalingAction::update(SpriteArray& array,float dt,ActionEventBuffer& buffer) {
		if (_buffer.size > 0) {
			// move
			for (int i = 0; i < _buffer.size; ++i) {
				array.scale(_ids[i], tweening::interpolate(_tweeningTypes[i], _startScale[i], _endScale[i], _timers[i], _ttl[i]));
				_timers[i] += dt;
				if ( _timers[i] >= _ttl[i] ) {
					if ( _modes[i] < 0 ) {
						_timers[i] = 0.0f;
					}
					else if ( _modes[i] == 0 ) {
						array.scale(_ids[i],_endScale[i]);
						buffer.add(_ids[i], AT_SCALE, array.getType(_ids[i]));
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
	void ScalingAction::debug() {
		if ( _buffer.size > 0 ) {
			LOG << "---------- ScalingAction ---------- ";
		}
		for ( int i = 0; i < _buffer.size; ++i ) {
			LOG << i << " : id: " << _ids[i] << " timer: " << _timers[i];
		}
		/*
		std::map<SID,int>::iterator it = m_Mapping.begin();
		while ( it != m_Mapping.end()) {
			LOG << it->first << " = " << it->second;
			++it;
		}
		*/
	}

}