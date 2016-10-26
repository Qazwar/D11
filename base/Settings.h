#pragma once
#include "core\math\math_types.h"
#include "core\io\FileRepository.h"
#include "core\graphics\Color.h"

namespace ds {

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
		bool reloading;
	
		Settings() {
			screenWidth = 1024;
			screenHeight = 768;
			clearColor = Color(100, 149, 237, 255);
			fullScreen = false;
			synched = true;
			reloading = true;
			mode = 1;
			initialMemorySize = 64;
			repositoryMode = repository::RM_DEBUG;
			reportingDirectory = "reports";
		}
	};

}