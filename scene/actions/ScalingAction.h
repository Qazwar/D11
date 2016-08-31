#pragma once
#include "..\..\renderer\render_types.h"
#include "core\math\tweening.h"
#include <map>
#include "..\Scene.h"
#include "AbstractAction.h"
#include "..\EntityArray.h"

namespace ds {

	class ScalingAction : public AbstractAction {
		
	public:
		ScalingAction();
		virtual ~ScalingAction();
		void attach(ID id,const v3& startScale,const v3& endScale,float ttl,int mode = 0,const tweening::TweeningType& tweeningType = &tweening::easeOutQuad);
		void update(EntityArray& array,float dt,ActionEventBuffer& buffer);
		void debug();
		void debug(ID sid) {}
		void save(const ReportWriter& writer);
		ActionType getActionType() const {
			return AT_SCALE;
		}
	private:
		void allocate(int sz);
		v3* _startScale;
		v3* _endScale;
		float* _timers;
		float* _ttl;
		tweening::TweeningType* _tweeningTypes;
		int* _modes;
	};

}