#pragma once
#include "..\..\renderer\render_types.h"
#include "..\..\math\tweening.h"
#include "..\World.h"
#include "AbstractAction.h"

namespace ds {

	class AlphaFadeToAction : public AbstractAction {

	public:
		AlphaFadeToAction();
		~AlphaFadeToAction();
		void attach(SID id,float startAlpha,float endAlpha,float ttl,int mode = 0,const tweening::TweeningType& tweeningType = &tweening::easeOutQuad);
		void update(SpriteArray& array,float dt,ActionEventBuffer& buffer);		
		void debug();
		void debug(SID sid) {}
		ActionType getActionType() const {
			return AT_ALPHA_FADE_TO;
		}
	private:
		void allocate(int sz);
		float* _startAlphas;
		float* _endAlphas;
		float* _timers;
		float* _ttl;
		tweening::TweeningType* _tweeningTypes;
		int* _modes;
	};


}