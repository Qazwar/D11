#pragma once
#include "..\..\renderer\render_types.h"
#include "..\..\math\tweening.h"
#include "..\World.h"
#include "AbstractAction.h"

namespace ds {

	class ColorFadeToAction : public AbstractAction {

	public:
		ColorFadeToAction();
		~ColorFadeToAction();
		void attach(SID id,const Color& startColor,const Color& endColor,float ttl,int mode = 0,const tweening::TweeningType& tweeningType = &tweening::easeOutQuad);
		void update(SpriteArray& array,float dt,ActionEventBuffer& buffer);		
		void debug();
		void debug(SID sid) {}
		ActionType getActionType() const {
			return AT_COLOR_FADE_TO;
		}
	private:
		void allocate(int sz);
		Color* _startColors;
		Color* _endColors;
		float* _timers;
		float* _ttl;
		tweening::TweeningType* _tweeningTypes;
		int* _modes;
	};


}