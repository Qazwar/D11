#pragma once
#include "..\Common.h"
#include "..\lib\collection_types.h"

namespace ds {

	enum ActionType {
		AT_ALPHA_FADE_TO,
		AT_COLOR_FADE_TO,
		AT_MOVE_TO,
		AT_MOVE_BY,
		AT_FOLLOW_PATH,
		AT_FOLLOW_CURVE,
		AT_FOLLOW_TARGET,
		AT_REMOVE_AFTER,
		AT_WAIT,
		AT_MOVE_WITH,
		AT_ROTATE,
		AT_ROTATE_BY,
		AT_FOLLOW_STRAIGHT_PATH,
		AT_KILL,
		AT_SCALE_BY_PATH,
		AT_SCALE,
		AT_COLOR_FLASH,
		AT_ROTATE_TO
	};

	struct ActionEvent {

		ID sid;
		ActionType type;
		int spriteType;
	};

	struct ActionEventBuffer {

		Array<ActionEvent> events;

		void reset() {
			events.clear();
		}

		void add(ID sid, ActionType type, int spriteType) {
			ActionEvent e;
			e.sid = sid;
			e.type = type;
			e.spriteType = spriteType;
			events.push_back(e);
		}
	};

}