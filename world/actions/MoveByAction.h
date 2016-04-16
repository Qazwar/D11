#pragma once
#include "..\..\renderer\render_types.h"
#include "..\World.h"
#include "AbstractAction.h"

namespace ds {

	class MoveByAction : public AbstractAction {

	public:
		MoveByAction();
		virtual ~MoveByAction() {}
		void attach(SID id,const Vector2f& velocity,bool bounce = false);
		void update(SpriteArray& array,float dt,ActionEventBuffer& buffer);
		void bounce(SpriteArray& array, SID sid, BounceDirection direction,float dt);
		void debug();
		void debug(SID sid);
		ActionType getActionType() const {
			return AT_MOVE_BY;
		}
		void save(const ReportWriter& writer);
	private:
		void allocate(int sz);
		void rotateTo(SpriteArray& array, int index);
		bool isOutOfBounds(const Vector2f& pos, const Vector2f& v);

		v2* _velocities;
		//float* _timers;
		bool* _bounce;
	};

}