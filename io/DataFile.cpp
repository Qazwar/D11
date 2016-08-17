#pragma once
#include "DataFile.h"
#include "..\utils\Profiler.h"
#include "..\utils\Log.h"
#include "json.h"
#include "FileRepository.h"

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
		LOG << "Reading simplified json file: " << buffer;
		JSONReader reader;
		if (reader.parse(buffer)) {
			ret = loadData(reader);
		}
		else {
			LOG << "Error: Cannot parse file: " << buffer;
			ret = false;
		}		
		s.end();
		LOG << "----> elapsed: " << s.elapsed();
		return ret;
	}

	// -----------------------------------------------
	// load JSON
	// -----------------------------------------------
	bool AssetFile::load() {
		bool ret = false;
		// check length
		StopWatch s;
		s.start();
		LOG << "Reading simplified json file: " << _name;
		JSONReader reader;
		if (reader.parse(_name)) {
			if (_loaded) {
				LOG << "-> Reloading";
				ret = reloadData(reader);
			}
			else {
				LOG << "-> Loading";
				ret = loadData(reader);
				if (ret) {
					_loaded = true;
				}
			}
			repository::add(this);
		}
		else {
			LOG << "Error: Cannot parse file: " << _name;
			ret = false;
		}
		s.end();
		LOG << "----> elapsed: " << s.elapsed();
		return ret;
	}

	// -----------------------------------------------
	// load JSON
	// -----------------------------------------------
	bool FlatAssetFile::load() {
		bool ret = false;
		// check length
		StopWatch s;
		s.start();
		LOG << "Reading simplified json file: " << _name;
		FlatJSONReader reader;
		if (reader.parse(_name)) {
			if (_loaded) {
				LOG << "-> Reloading";
				ret = reloadData(reader);
			}
			else {
				LOG << "-> Loading";
				ret = loadData(reader);
				if (ret) {
					_loaded = true;
				}
			}
			repository::add(this);
		}
		else {
			LOG << "Error: Cannot parse file: " << _name;
			ret = false;
		}
		s.end();
		LOG << "----> elapsed: " << s.elapsed();
		return ret;
	}


}