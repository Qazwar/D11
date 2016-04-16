#pragma once
#include "..\..\renderer\render_types.h"
#include "..\..\math\tweening.h"
#include <map>
#include "..\World.h"
#include "..\..\math\tweening.h"
#include "AbstractAction.h"
#include "..\..\math\BezierCurve.h"

namespace ds {

	class FollowCurveAction : public AbstractAction {

	public:
		FollowCurveAction();
		virtual ~FollowCurveAction();
		void attach(SID id,BezierCurve* path,float ttl,int mode = 0);
		void update(SpriteArray& array,float dt,ActionEventBuffer& buffer);
		void debug();
		void debug(SID sid) {}
		ActionType getActionType() const {
			return AT_FOLLOW_CURVE;
		}
	private:
		void allocate(int sz);
		BezierCurve** _path;
		float* _timers;
		float* _ttl;
		int* _modes;
	};

}