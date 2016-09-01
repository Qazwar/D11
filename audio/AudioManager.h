#pragma once
#include <dsound.h>
#include "core\Common.h"
#include "core\lib\collection_types.h"
#include "core\string\StaticHash.h"

namespace ds {

	namespace audio {

		bool initialize(HWND hwnd);

		void shutdown();

		int load(const StaticHash& hash);

		void play(int id);

		void mix();

	}

};

