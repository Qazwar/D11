#pragma once
#include "..\..\renderer\render_types.h"
#include "core\math\tweening.h"
#include "..\Scene.h"
#include "AbstractAction.h"

namespace ds {

	class MoveToAction : public AbstractAction {

	public:
		MoveToAction();
		virtual ~MoveToAction() {}
		void attach(ID id, EntityArray& array,const v3& startPos, const v3& endPos, float ttl, int mode = 0, const tweening::TweeningType& tweeningType = &tweening::linear);
		void update(EntityArray& array,float dt,ActionEventBuffer& buffer);
		void debug();
		void debug(ID sid);
		void save(const ReportWriter& writer);
		ActionType getActionType() const {
			return AT_MOVE_TO;
		}
	private:
		void allocate(int sz);
		v3* _startPositions;
		v3* _endPositions;
		float* _timers;
		float* _ttl;
		tweening::TweeningType* _tweeningTypes;
		int* _modes;
	};

}