#pragma once
#include "..\editor\EditorPlugin.h"

namespace ds {

	class PerfHUDPlugin : public EditorPlugin {

	public:
		PerfHUDPlugin() : EditorPlugin("PerfHUD") {}
		void showDialog();
	};
}
