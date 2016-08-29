#include "FileRepository.h"
#include "..\utils\StringUtils.h"
#include "..\utils\Log.h"
#include "TextCompressor.h"
#include "..\lib\collection_types.h"
#include "DataFile.h"
#include "..\utils\FileUtils.h"
#include "..\utils\GlobalStringBuffer.h"
#include "..\utils\StaticHash.h"

namespace ds {

	namespace repository {

		// -----------------------------------------------------------
		// FileInfo
		// -----------------------------------------------------------
		struct FileInfo {
			StaticHash hash;
			DataFile* dataFile;
			FILETIME filetime;
		};

		// -----------------------------------------------------------
		// RepositoryEntry
		// -----------------------------------------------------------
		struct RepositoryEntry {
			int name_index;
			StaticHash hash;
			int index;
			int size;
			bool encoded;
		};

		// -----------------------------------------------------------
		// FileRepo
		// -----------------------------------------------------------
		struct FileRepo {

			RepositoryMode mode;
			Array<RepositoryEntry> entries;
			Array<FileInfo> infos;
			CharBuffer name_buffer;
		};

		static FileRepo* _repository = 0;

		// -----------------------------------------------------------
		// intialize
		// -----------------------------------------------------------
		void initialize(RepositoryMode mode) {
			_repository = new FileRepo;
			_repository->mode = mode;
			if (mode == RM_RELEASE) {
				FILE* f = fopen("data\\e.pak", "rb");
				if (f) {
					int sz = 0;
					fread(&sz, sizeof(int), 1, f);
					LOG << "repository entries: " << sz;
					for ( int i = 0; i < sz; ++i ) {
						RepositoryEntry entry;
						fread(&entry.hash, sizeof(StaticHash), 1, f);
						fread(&entry.size, sizeof(int), 1, f);
						fread(&entry.index, sizeof(int), 1, f);
						fread(&entry.encoded, sizeof(bool), 1, f);
						_repository->entries.push_back(entry);
					}
					fclose(f);
				}
			}
		}

		// -----------------------------------------------------------
		// already loaded
		// -----------------------------------------------------------
		bool already_loaded(const char* fileName) {
			StaticHash hash = StaticHash(fileName);
			for (size_t i = 0; i < _repository->entries.size(); ++i) {
				const RepositoryEntry& entry = _repository->entries[i];
				if (entry.hash == hash) {
					return true;
				}
			}
			return false;
		}

		// -----------------------------------------------------------
		// load and store file info
		// -----------------------------------------------------------
		void load(DataFile* file, FileType type) {
			int size = 0;
			char buffer[256];
			if (file->load()) {
				sprintf_s(buffer, 256, "content\\%s", file->getFileName());
				FileInfo info;
				file::getFileTime(buffer, info.filetime);
				_repository->infos.push_back(info);
			}
		}

		// -----------------------------------------------------------
		// already in the watch list
		// -----------------------------------------------------------
		bool already_watching(const char* fileName) {
			StaticHash hash = StaticHash(fileName);
			for (size_t i = 0; i < _repository->infos.size(); ++i) {
				const FileInfo& entry = _repository->infos[i];
				if (entry.hash == hash) {
					return true;
				}
			}
			return false;
		}

		void add(DataFile* file) {
			if (!already_watching(file->getFileName())) {
				int size = 0;
				FileInfo info;
				info.dataFile = file;
				info.hash = StaticHash(file->getFileName());
				file::getFileTime(file->getFileName(), info.filetime);
				_repository->infos.push_back(info);
			}
		}

		// -----------------------------------------------------------
		// reload
		// -----------------------------------------------------------
		void reload() {
			int reloaded = 0;
			for (uint32_t i = 0; i < _repository->infos.size(); ++i) {
				FileInfo& info = _repository->infos[i];
				const char* name = info.dataFile->getFileName();
				if (file::compareFileTime(name, info.filetime)) {
					HANDLE hData = CreateFile(name, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
					if (hData != INVALID_HANDLE_VALUE) {
						CloseHandle(hData);
						LOG << "Reloading file: " << name;
						info.dataFile->load();
						file::getFileTime(name, info.filetime);
						++reloaded;						
					}
					else {
						LOGE << "Cannot read file: '" << name << "'";
					}
				}
			}
			if (reloaded > 0) {
				LOG << "Number of reloaded files: " << reloaded;
			}
		}

		// -----------------------------------------------------------
		// load
		// -----------------------------------------------------------
		char* load(const char* fileName, int* size, FileType type) {
			//XASSERT(_repository != 0,"No repository created yet");
			*size = -1;
			if (_repository->mode == RM_DEBUG) {
				FILE *fp = fopen(fileName, "rb");
				if (fp) {
					LOG << "Loading '" << fileName << "'";
					fseek(fp, 0, SEEK_END);
					int sz = ftell(fp);
					fseek(fp, 0, SEEK_SET);
					LOG << "size: " << sz;
					char* buffer = new char[sz + 1];
					fread(buffer, 1, sz, fp);
					buffer[sz] = '\0';
					fclose(fp);
					*size = sz;
					if (!already_loaded(fileName)) {
						RepositoryEntry entry;
						entry.hash = StaticHash(fileName);
						entry.name_index = _repository->name_buffer.append(fileName);
						entry.size = sz;
						entry.index = -1;
						if (type == FT_TEXT) {
							entry.encoded = true;
						}
						else {
							entry.encoded = false;
						}
						_repository->entries.push_back(entry);
					}
					return buffer;
				}
				else {
					LOGE << "Cannot find file: '" << fileName << "'";
					return 0;
				}
			}
			else {
				RepositoryEntry* selected = 0;
				StaticHash hash = StaticHash(fileName);
				for (size_t i = 0; i < _repository->entries.size(); ++i) {
					const RepositoryEntry& entry = _repository->entries[i];
					if (entry.hash == hash) {
						selected = &_repository->entries[i];
					}
				}
				if (selected != 0) {
					FILE* fp = fopen("data\\c.pak","rb");
					if (fp) {
						LOG << "reading at " << selected->index << " size: " << selected->size;
						fseek(fp, selected->index, SEEK_SET);
						char* buffer = new char[selected->size];
						fread(buffer, 1, selected->size, fp);
						if (selected->encoded) {
							LOG << "file is encoded";
							char* result = compression::decode(buffer);
							fclose(fp);
							delete[] buffer;
							*size = selected->size;
							return result;
						}
						else {
							LOG << "raw data";
							*size = selected->size;
							return buffer;
						}
					}
					else {
						LOGE << "Cannot find content PAK";
						return 0;
					}
				}
				else {
					LOGE << "Cannot find matching entry for '" << fileName << "'";
				}
				return 0;
			}
			return 0;
		}

		// -----------------------------------------------------------
		// read file into char buffer
		// -----------------------------------------------------------
		char* read_file(const char* fileName,int* fileSize) {
			FILE *fp = fopen(fileName, "rb");
			if (fp) {
				fseek(fp, 0, SEEK_END);
				int sz = ftell(fp);
				fseek(fp, 0, SEEK_SET);
				char* buffer = new char[sz + 1];
				fread(buffer, 1, sz, fp);
				//buffer[sz] = '\0';
				fclose(fp);
				*fileSize = sz;
				return buffer;
			}
			return 0;
		}

		// -----------------------------------------------------------
		// shutdown
		// -----------------------------------------------------------
		void shutdown() {
			/*
			if (_repository->mode == RM_DEBUG) {
				LOG << "entries: " << _repository->entries.size();
				FILE* fp = fopen("data\\c.pak", "wb");
				int index = 0;
				for (size_t i = 0; i < _repository->entries.size(); ++i) {
					RepositoryEntry& entry = _repository->entries[i];										
					entry.index = index;
					// read file
					int fileSize = -1;
					char* content = read_file(entry.name, &fileSize);
					LOG << "adding file: " << entry.name << " file size: " << fileSize;
					if (content != 0) {
						// encode
						if (entry.encoded) {
							LOG << "file will be encoded";
							int size = -1;
							char* encoded = compression::encode(content, &size);
							delete[] content;
							int counter = 0;
							// write buffer
							for (int j = 0; j < size; ++j) {
								fputc(encoded[j], fp);
								++counter;
							}
							entry.size = counter;
							index += counter;
							delete[] encoded;
						}
						else {
							LOG << "saving raw data";
							int counter = 0;
							// write buffer
							for (int j = 0; j < fileSize; ++j) {
								fputc(content[j], fp);
								++counter;
							}
							entry.size = counter;
							index += counter;
							delete[] content;
						}
					}
					else {
						LOGE << "Cannot find file: '" << entry.name << "'";
					}
				}
				fclose(fp);
				// save directory
				FILE* f = fopen("data\\e.pak", "wb");
				if (f) {
					int sz = _repository->entries.size();
					fwrite(&sz, sizeof(int), 1, f);
					for (size_t i = 0; i < _repository->entries.size(); ++i) {
						const RepositoryEntry& entry = _repository->entries[i];
						LOG << i << " - name: '" << entry.name << "' index: " << entry.index;
						fwrite(&entry.hash, sizeof(IdString), 1, f);
						fwrite(&entry.size, sizeof(int), 1, f);
						fwrite(&entry.index, sizeof(int), 1, f);
						fwrite(&entry.encoded, sizeof(bool), 1, f);
					}
					fclose(f);
				}
			}
			*/
			delete _repository;
		}

		// -----------------------------------------------------------
		// list entries of e.pak
		// -----------------------------------------------------------
		void list() {
			Array<RepositoryEntry> entries;
			FILE* f = fopen("data\\e.pak", "rb");
			if (f) {
				int sz = 0;
				fread(&sz, sizeof(int), 1, f);
				for (int i = 0; i < sz; ++i) {
					RepositoryEntry entry;
					fread(&entry.hash, sizeof(StaticHash), 1, f);
					fread(&entry.size, sizeof(int), 1, f);
					fread(&entry.index, sizeof(int), 1, f);
					entries.push_back(entry);
				}
				fclose(f);
			}
			for (size_t i = 0; i < entries.size(); ++i) {
				const RepositoryEntry& entry = entries[i];
				const char* name = _repository->name_buffer.data + entry.name_index;
				LOG << name << " size: " << entry.size << " index: " << entry.index;
				FILE* fp = fopen("data\\c.pak", "rb");
				if (fp) {
					fseek(fp, entry.index, SEEK_SET);
					char* buffer = new char[entry.size + 1];
					fread(buffer, 1, entry.size, fp);
					char* result = compression::decode(buffer);
					int len = strlen(result);
					fclose(fp);
					delete[] buffer;
					LOG << "##### len: " << len;
					LOG << "RESULT: '" << result << "'";
					delete[] result;
				}
			}
		}
	}

	

}
