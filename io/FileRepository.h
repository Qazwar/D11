#pragma once

namespace ds {

	class DataFile;
	class AssetFile;

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

		char* load(const char* fileName, int* size, FileType type = FT_TEXT);

		void load(DataFile* file, FileType type = FT_TEXT);

		void add(DataFile* file);

		void reload();

		void list();
	}

}

