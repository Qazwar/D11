#include "AlphaFadeToAction.h"
#include "..\..\math\GameMath.h"
#include "..\..\utils\Log.h"

namespace ds {
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	AlphaFadeToAction::AlphaFadeToAction() : AbstractAction("alpha_fade") {
		int sizes[] = { sizeof(SID), sizeof(float), sizeof(float),sizeof(float),sizeof(float),sizeof(tweening::TweeningType),sizeof(int) };
		_buffer.init(sizes, 7);
	}

	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	AlphaFadeToAction::~AlphaFadeToAction() {}

	void AlphaFadeToAction::allocate(int sz) {
		if (_buffer.resize(sz)) {
			_ids = (SID*)_buffer.get_ptr(0);
			_startAlphas = (float*)_buffer.get_ptr(1);
			_endAlphas = (float*)_buffer.get_ptr(2);
			_timers = (float*)_buffer.get_ptr(3);
			_ttl = (float*)_buffer.get_ptr(4);
			_tweeningTypes = (tweening::TweeningType*)_buffer.get_ptr(5);
			_modes = (int*)_buffer.get_ptr(6);
		}
	}
	// -------------------------------------------------------
	// 
	// -------------------------------------------------------
	void AlphaFadeToAction::attach(SID id,float startAlpha,float endAlpha,float ttl,int mode,const tweening::TweeningType& tweeningType) {
		int idx = create(id);
		_ids[idx] = id;
		_startAlphas[idx] = startAlpha;
		_endAlphas[idx] = endAlpha;
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
	void AlphaFadeToAction::update(SpriteArray& array,float dt,ActionEventBuffer& buffer) {	
		if ( _buffer.size > 0 ) {				
			// move
			for ( int i = 0; i < _buffer.size; ++i ) {
				float alpha = tweening::interpolate(_tweeningTypes[i], _startAlphas[i], _endAlphas[i], _timers[i], _ttl[i]);
				array.setAlpha(_ids[i],alpha);
				_timers[i] += dt;
				if ( _timers[i] >= _ttl[i] ) {
					if ( _modes[i] < 0 ) {
						_timers[i] = 0.0f;
					}
					else if ( _modes[i] == 0 ) {
						array.setAlpha(_ids[i], _endAlphas[i]);
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
	void AlphaFadeToAction::debug() {
		for ( int i = 0; i < _buffer.size; ++i ) {
			LOG << i << " : id: " <<_ids[i] << " timer: " << _timers[i];
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