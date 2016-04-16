#pragma once
#include "..\..\renderer\render_types.h"
#include "..\..\math\tweening.h"
#include "..\World.h"
#include "AbstractAction.h"

namespace ds {

	class RotateByAction : public AbstractAction {

	public:
		RotateByAction();
		virtual ~RotateByAction() {}
		void attach(SID id,float startAngle,float endAngle,float ttl,int mode = 0,const tweening::TweeningType& tweeningType = &tweening::easeOutQuad);
		void update(SpriteArray& array,float dt,ActionEventBuffer& buffer);
		void debug();
		void debug(SID sid);
		ActionType getActionType() const {
			return AT_ROTATE_BY;
		}
	private:
		void allocate(int sz);
		float* _startAngles;
		float* _endAngles;
		float* _steps;
		float* _timers;
		float* _ttl;
		tweening::TweeningType* _tweeningTypes;
		int* _modes;
	};

}