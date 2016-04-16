#pragma once
#include "..\..\renderer\render_types.h"
#include "..\World.h"
#include "AbstractAction.h"

namespace ds {

	class RemoveAfterAction : public AbstractAction {

	public:
		RemoveAfterAction();
		virtual ~RemoveAfterAction() {}
		void attach(SID id, float ttl);
		void update(SpriteArray& array,float dt,ActionEventBuffer& buffer);
		void debug();
		void debug(SID sid);
		ActionType getActionType() const {
			return AT_REMOVE_AFTER;
		}
	private:
		void allocate(int sz);
		float* _timers;
		float* _ttl;
	};

}