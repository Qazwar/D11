#pragma once
#include "..\..\renderer\render_types.h"
#include "..\..\math\tweening.h"
#include <map>
#include "..\World.h"
#include "..\..\math\tweening.h"
#include "AbstractAction.h"
#include "..\..\math\CubicBezierPath.h"

namespace ds {

	class FollowPathAction : public AbstractAction {

	public:
		FollowPathAction();
		virtual ~FollowPathAction();
		void attach(SID id,CubicBezierPath* path,float ttl,int mode = 0);
		void update(SpriteArray& array,float dt,ActionEventBuffer& buffer);
		void debug();
		void debug(SID sid) {}
		ActionType getActionType() const {
			return AT_FOLLOW_PATH;
		}
	private:
		void allocate(int sz);
		CubicBezierPath** _path;
		float* _timers;
		float* _ttl;
		int* _modes;
	};

}