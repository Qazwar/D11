#pragma once
#include <stdint.h>
#include "..\utils\StaticHash.h"

namespace ds {

	class DataFile;

	namespace repository {

		enum RepositoryMode {
			RM_DEBUG,
			RM_RELEASE
		};

		enum FileType {
			FT_TEXT,
			FT_BINARY,
			FT_EOL
		};

		void initialize(RepositoryMode mode);

		void shutdown();

		char* load(const StaticHash& hash, int* size, FileType type = FT_TEXT);

		void load(DataFile* file, FileType type = FT_TEXT);

		void add(DataFile* file);

		void reload();

		void list();
	}

}

