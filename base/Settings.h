#pragma once
#include "..\utils\Color.h"
#include "..\io\FileRepository.h"

namespace ds {

	enum LogTypes {
		LT_NONE,
		LT_CONSOLE,
		LT_FILE
	};

	struct Settings {

		Color clearColor;
		int screenWidth;
		int screenHeight;
		bool fullScreen;
		bool synched;
		int mode;
		const char* reportingDirectory;
		int initialMemorySize;
		repository::RepositoryMode repositoryMode;
		int logTypes;

		Settings() {
			screenWidth = 1024;
			screenHeight = 768;
			clearColor = Color(100, 149, 237, 255);
			fullScreen = false;
			synched = true;
			mode = 1;
			initialMemorySize = 64;
			repositoryMode = repository::RM_DEBUG;
			reportingDirectory = "reports";
			logTypes = 0;
		}
	};

}