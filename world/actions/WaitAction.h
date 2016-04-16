#pragma once
#include "..\..\renderer\render_types.h"
#include "..\World.h"
#include "AbstractAction.h"

namespace ds {

	class WaitAction : public AbstractAction {

	public:
		WaitAction();
		virtual ~WaitAction();
		void attach(SID id,float ttl);
		void update(SpriteArray& array,float dt,ActionEventBuffer& buffer);
		void debug();
		void debug(SID sid);
		ActionType getActionType() const {
			return AT_WAIT;
		}
	private:
		void allocate(int sz);		
		float* _timers;
		float* _ttl;
	};

}