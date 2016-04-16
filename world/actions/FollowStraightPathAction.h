#pragma once
#include "..\..\renderer\render_types.h"
#include "..\..\math\tweening.h"
#include <map>
#include "..\World.h"
#include "..\..\math\tweening.h"
#include "AbstractAction.h"
#include "..\..\math\StraightPath.h"

namespace ds {

	class FollowStraightPathAction : public AbstractAction {

	public:
		FollowStraightPathAction();
		virtual ~FollowStraightPathAction();
		void attach(SID id, SpriteArray& array,StraightPath* path, float ttl, int mode = 0);
		void update(SpriteArray& array,float dt,ActionEventBuffer& buffer);
		void debug();
		void debug(SID sid) {}
		ActionType getActionType() const {
			return AT_FOLLOW_STRAIGHT_PATH;
		}
	private:
		void allocate(int sz);
		StraightPath** _path;
		float* _timers;
		float* _ttl;
		int* _modes;
	};

}