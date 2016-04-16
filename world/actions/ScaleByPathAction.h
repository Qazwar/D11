#pragma once
#include "..\..\renderer\render_types.h"
#include "..\World.h"
#include "AbstractAction.h"
#include "..\..\math\FloatArray.h"

namespace ds {

	struct ScaleByPathDefinition : public AbstractActionDefinition {

		Vector2fPath path;
		float ttl;

		ActionType getActionType() const {
			return AT_SCALE_BY_PATH;
		}


		void read(const JSONReader& reader, int category_id) {
			//reader.get_vec2_path(category_id, "path", &path);
			//reader.get_float(category_id, "ttl", &ttl);
		}
	};

	class ScaleByPathAction : public AbstractAction {

	public:
		ScaleByPathAction();
		virtual ~ScaleByPathAction();
		void attach(SpriteArray& array,SID id, Vector2fPath* path, float ttl);
		void attach(SID id, AbstractActionDefinition* definition);
		void update(SpriteArray& array,float dt,ActionEventBuffer& buffer);
		void debug();
		void debug(SID sid) {}
		ActionType getActionType() const {
			return AT_SCALE_BY_PATH;
		}
		void save(const ReportWriter& writer);
		AbstractActionDefinition* createDefinition() const {
			return new ScaleByPathDefinition();
		}
	protected:
		SID swap(int i);
	private:
		void allocate(int sz);
		Vector2fPath** _path;
		float* _timers;
		float* _ttl;
	};

}