#pragma once
#include "..\..\renderer\render_types.h"
#include "..\..\math\tweening.h"
#include <map>
#include "..\World.h"
#include "..\..\math\tweening.h"
#include "AbstractAction.h"

namespace ds {

	
	class MoveWithAction : public AbstractAction {

	public:
		MoveWithAction();
		virtual ~MoveWithAction() {}
		void attach(SID id,const MoveFunc& function,float ttl,int mode = 0);
		void update(SpriteArray& array,float dt,ActionEventBuffer& buffer);
		void debug();
		void debug(SID sid) {}
		ActionType getActionType() const {
			return AT_MOVE_WITH;
		}
	private:
		void allocate(int sz);
		MoveFunc* _functions;
		float* _timers;
		float* _ttl;
		int* _modes;
	};

}