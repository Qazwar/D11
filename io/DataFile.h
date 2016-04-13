#pragma once
#include "..\io\json.h"
#include "..\utils\Profiler.h"

namespace ds {

	class DataFile {

	public:
		DataFile() {}
		virtual ~DataFile() {}
		virtual bool saveData(JSONWriter& writer) = 0;
		virtual bool loadData(const JSONReader& loader) = 0;
		virtual const char* getFileName() const = 0;
		bool save();		
		bool load();
	};

}