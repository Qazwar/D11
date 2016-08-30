#pragma once
#include "..\io\json.h"
#include "..\utils\Profiler.h"
#include <assert.h>

namespace ds {

	class DataFile {

	public:
		DataFile(const char* fileName) : _loaded(false) {
			_hash = StaticHash(fileName);
		}
		virtual ~DataFile() {}
		const StaticHash& getFileName() const {
			return _hash;
		}
		virtual bool load() = 0;
		bool isLoaded() const {
			return _loaded;
		}
	protected:
		bool _loaded;
		StaticHash _hash;
	};

	class JSONAssetFile : public DataFile {

	public:
		JSONAssetFile(const char* name) : DataFile(name) {}
		virtual ~JSONAssetFile() {}
		virtual bool loadData(const JSONReader& loader) = 0;
		virtual bool reloadData(const JSONReader& loader) = 0;
		bool load();
	};

	class FlatJSONAssetFile : public DataFile {

	public:
		FlatJSONAssetFile(const char* name) : DataFile(name) {}
		virtual ~FlatJSONAssetFile() {}
		virtual bool loadData(const FlatJSONReader& loader) = 0;
		virtual bool reloadData(const FlatJSONReader& loader) = 0;
		bool load();
	};

}