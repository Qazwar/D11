#pragma once
#include "..\utils\Color.h"

namespace ds {

	struct Settings {

		Color clearColor;
		int screenWidth;
		int screenHeight;
		bool fullScreen;
		bool synched;
		int mode;
		char reportingDirectory[64];

		Settings() {
			screenWidth = 1024;
			screenHeight = 768;
			clearColor = Color(100, 149, 237, 255);
			fullScreen = false;
			synched = true;
			mode = 1;
		}
	};

}