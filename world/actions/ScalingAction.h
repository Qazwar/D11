#pragma once
#include "..\..\renderer\render_types.h"
#include "..\..\math\tweening.h"
#include <map>
#include "..\World.h"
#include "..\..\math\tweening.h"
#include "AbstractAction.h"

namespace ds {

	class ScalingAction : public AbstractAction {
		
	public:
		ScalingAction();
		virtual ~ScalingAction();
		void attach(SID id,const Vector2f& startScale,const Vector2f& endScale,float ttl,int mode = 0,const tweening::TweeningType& tweeningType = &tweening::easeOutQuad);
		void update(SpriteArray& array,float dt,ActionEventBuffer& buffer);
		void debug();
		void debug(SID sid) {}
		ActionType getActionType() const {
			return AT_SCALE;
		}
	private:
		void allocate(int sz);
		v2* _startScale;
		v2* _endScale;
		float* _timers;
		float* _ttl;
		tweening::TweeningType* _tweeningTypes;
		int* _modes;
	};

}