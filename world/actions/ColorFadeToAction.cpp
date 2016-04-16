#include "ColorFadeToAction.h"
#include "..\..\math\GameMath.h"
#include "..\..\utils\Log.h"

namespace ds {
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	ColorFadeToAction::ColorFadeToAction() : AbstractAction("color_fade") {
		int sizes[] = { sizeof(SID), sizeof(Color), sizeof(Color), sizeof(float), sizeof(float),sizeof(tweening::TweeningType),sizeof(int) };
		_buffer.init(sizes, 7);
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	ColorFadeToAction::~ColorFadeToAction() {}

	void ColorFadeToAction::allocate(int sz) {
		if (_buffer.resize(sz)) {
			_ids = (SID*)_buffer.get_ptr(0);
			_startColors = (Color*)_buffer.get_ptr(1);
			_endColors = (Color*)_buffer.get_ptr(2);
			_timers = (float*)_buffer.get_ptr(3);
			_ttl = (float*)_buffer.get_ptr(4);
			_tweeningTypes = (tweening::TweeningType*)_buffer.get_ptr(5);
			_modes = (int*)_buffer.get_ptr(6);
		}
	}
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void ColorFadeToAction::attach(SID id, const Color& startColor, const Color& endColor, float ttl, int mode, const tweening::TweeningType& tweeningType) {
		int idx = create(id);
		_ids[idx] = id;
		_startColors[idx] = startColor;
		_endColors[idx] = endColor;
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
	void ColorFadeToAction::update(SpriteArray& array, float dt, ActionEventBuffer& buffer) {
		if ( _buffer.size > 0 ) {				
			// move
			for ( int i = 0; i < _buffer.size; ++i ) {
				array.setColor(_ids[i], tweening::interpolate(_tweeningTypes[i], _startColors[i], _endColors[i], _timers[i], _ttl[i]));
				_timers[i] += dt;
				if ( _timers[i] >= _ttl[i] ) {
					if ( _modes[i] < 0 ) {
						_timers[i] = 0.0f;
					}
					else if ( _modes[i] == 0 ) {
						array.setColor(_ids[i], _endColors[i]);
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
	void ColorFadeToAction::debug() {
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