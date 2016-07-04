#pragma once
#include "..\..\renderer\render_types.h"
#include "..\Scene.h"
#include "AbstractAction.h"

namespace ds {

	class RotateToAction : public AbstractAction {

	public:
		RotateToAction();
		virtual ~RotateToAction() {}
		void attach(ID id, const v3& startAngle, const v3& endAngle, float ttl, int mode = 0, const tweening::TweeningType& tweeningType = &tweening::linear);
		void update(EntityArray& array,float dt,ActionEventBuffer& buffer);
		void debug();
		void debug(ID sid);
		void save(const ReportWriter& writer);
		ActionType getActionType() const {
			return AT_ROTATE_TO;
		}
	private:
		void allocate(int sz);
		v3* _startAngles;
		v3* _endAngles;
		float* _timers;
		float* _ttl;		
		tweening::TweeningType* _tweeningTypes;
		int* _modes;
	};

}