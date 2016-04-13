#pragma once
#include "DataFile.h"
#include "..\utils\Profiler.h"
#include "..\utils\Log.h"
#include "json.h"

namespace ds {

	// -----------------------------------------------
	// export JSON
	// -----------------------------------------------
	bool DataFile::save() {
		char buffer[64];
		sprintf_s(buffer, 64, "content\\%s", getFileName());
		LOG << "exporting simplified json file: " << buffer;
		// FIXME: check if directory exists
		JSONWriter jw;
		if (jw.open(buffer)) {
			return saveData(jw);
		}
		else {
			LOG << "Cannot open file";
			return false;
		}
	}
	
	// -----------------------------------------------
	// load JSON
	// -----------------------------------------------
	bool DataFile::load() {
		bool BINARY = false;
		bool ret = false;
		char buffer[64];
		// check length
		StopWatch s;
		s.start();
		sprintf_s(buffer, 64, "content\\%s", getFileName());
		// FIXME: assert that file name contains .
		LOG << "Reading simplified json file: " << buffer;
		JSONReader reader;
		if (BINARY) {
			sprintf_s(buffer, 64, "assets\\%u", string::murmur_hash(getFileName()));
			if (reader.load_binary(buffer)) {
				ret = loadData(reader);
			}
		}
		else {
			if (reader.parse(buffer)) {
				ret = loadData(reader);
			}
			if (ret) {
				sprintf_s(buffer, 64, "assets\\%u", string::murmur_hash(getFileName()));
				LOG << "saving binary file: " << buffer;
				reader.save_binary(buffer);
			}
		}
		if (!ret) {
			LOG << "Error: Cannot parse file: " << buffer;
			ret = false;
		}		
		s.end();
		LOG << "----> elapsed: " << s.elapsed();
		return ret;
	}


}