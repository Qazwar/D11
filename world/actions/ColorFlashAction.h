#pragma once
#include "..\..\renderer\render_types.h"
#include "..\..\math\tweening.h"
#include "..\World.h"
#include "AbstractAction.h"
#include <chrono>
#include "..\..\utils\Log.h"


namespace ds {

	struct ColorFlashDefinition : public AbstractActionDefinition {
		Color start;
		Color end;
		float ttl;
		tweening::TweeningType tweening;
		int mode;

		ActionType getActionType() const {
			return AT_COLOR_FLASH;
		}
		

		void read(const JSONReader& reader, int category_id) {
			reader.get_color(category_id, "start_color", &start);
			reader.get_color(category_id, "end_color", &end);
			int type = reader.get_int(category_id, "tweening", &type);
			tweening = tweening::get_by_index(type);
			reader.get_float(category_id, "ttl", &ttl);
			reader.get_int(category_id, "mode", &mode);
		}
	};

	class ColorFlashAction : public AbstractAction {

	public:
		ColorFlashAction();
		~ColorFlashAction();
		void attach(SID id,const Color& startColor,const Color& endColor,float ttl,int mode = 0,const tweening::TweeningType& tweeningType = &tweening::easeOutQuad);
		void attach(SID id, AbstractActionDefinition* definition);
		void update(SpriteArray& array,float dt,ActionEventBuffer& buffer);		
		void debug();
		void debug(SID sid) {}
		ActionType getActionType() const {
			return AT_COLOR_FLASH;
		}
		AbstractActionDefinition* createDefinition() const {
			return new ColorFlashDefinition();
		}
	private:
		void allocate(int sz);
		Color* _startColors;
		Color* _endColors;
		float* _timers;
		float* _ttl;
		tweening::TweeningType* _tweeningTypes;
		int* _modes;
	};


}