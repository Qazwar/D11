#pragma once
#include "DataFile.h"
#include "..\utils\Profiler.h"
#include "..\utils\Log.h"
#include "json.h"
#include "FileRepository.h"

namespace ds {

	
	// -----------------------------------------------
	// load JSON
	// -----------------------------------------------
	bool JSONAssetFile::load() {
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
	bool FlatJSONAssetFile::load() {
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