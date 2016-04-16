#pragma once
#include "..\..\renderer\render_types.h"
#include "..\..\math\tweening.h"
#include "..\World.h"
#include "..\..\math\tweening.h"
#include "AbstractAction.h"

namespace ds {

	class MoveToAction : public AbstractAction {

	public:
		MoveToAction();
		virtual ~MoveToAction() {}
		void attach(SID id, SpriteArray& array,const Vector2f& startPos, const Vector2f& endPos, float ttl, int mode = 0, const tweening::TweeningType& tweeningType = &tweening::easeOutQuad);
		void update(SpriteArray& array,float dt,ActionEventBuffer& buffer);
		void debug();
		void debug(SID sid);
		ActionType getActionType() const {
			return AT_MOVE_TO;
		}
	private:
		void allocate(int sz);
		v2* _startPositions;
		v2* _endPositions;
		float* _timers;
		float* _ttl;
		tweening::TweeningType* _tweeningTypes;
		int* _modes;
	};

}