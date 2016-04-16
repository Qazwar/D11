#pragma once
#include "..\..\renderer\render_types.h"
#include "..\..\math\tweening.h"
#include <map>
#include "..\World.h"
#include "..\..\math\tweening.h"
#include "AbstractAction.h"
#include "..\..\math\CubicBezierPath.h"

namespace ds {

	class FollowTargetAction : public AbstractAction {

	public:
		FollowTargetAction();
		virtual ~FollowTargetAction();
		void attach(SID id,SID target,float velocity);
		void update(SpriteArray& array,float dt,ActionEventBuffer& buffer);
		void debug();
		void debug(SID sid) {}
		ActionType getActionType() const {
			return AT_FOLLOW_TARGET;
		}
	private:
		int computeSeparation(SpriteArray& array, int self, float radius, Vector2f* v);
		int findNearest(SpriteArray& array, const Vector2f& pos, float radius, int self, int type);
		void allocate(int sz);
		SID* _targets;
		float* _velocities;
	};

}