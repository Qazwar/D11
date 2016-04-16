#pragma once
#include "..\..\renderer\render_types.h"
#include "..\World.h"
#include "AbstractAction.h"

namespace ds {

	class RotateAction : public AbstractAction {

	public:
		RotateAction();
		virtual ~RotateAction() {}
		void attach(SID id,float angleVelocity,float ttl,int mode = 0);
		void update(SpriteArray& array,float dt,ActionEventBuffer& buffer);
		void debug();
		void debug(SID sid);
		ActionType getActionType() const {
			return AT_ROTATE;
		}
	private:
		void allocate(int sz);
		float* _angleVelocities;
		float* _timers;
		float* _ttl;
		int* _modes;
	};

}