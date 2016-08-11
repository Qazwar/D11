#pragma once
#include "DynamicSettings.h"
#include "..\utils\Log.h"

namespace ds {

	

	DynamicGameSettings::DynamicGameSettings() {

	}

	DynamicGameSettings::~DynamicGameSettings() {

	}

	void DynamicGameSettings::add(const char* name, float* value, float defaultValue) {
		*value = defaultValue;
		SettingsItem item;
		item.name = name;
		item.hash = string::murmur_hash(name);
		item.type = ST_FLOAT;
		item.ptr.fPtr = value;
		items.push_back(item);
	}

	void DynamicGameSettings::add(const char* name, int* value, int defaultValue) {
		*value = defaultValue;
		SettingsItem item;
		item.name = name;
		item.hash = string::murmur_hash(name);
		item.type = ST_INT;
		item.ptr.iPtr = value;
		items.push_back(item);
	}

	void DynamicGameSettings::add(const char* name, v2* value, const v2& defaultValue) {
		*value = defaultValue;
		SettingsItem item;
		item.name = name;
		item.hash = string::murmur_hash(name);
		item.type = ST_VEC2;
		item.ptr.v2Ptr = value;
		items.push_back(item);
	}

	// -----------------------------------------------
	// load JSON
	// -----------------------------------------------
	bool DynamicGameSettings::load() {
		bool BINARY = false;
		bool ret = false;
		char buffer[64];
		// check length
		StopWatch s;
		s.start();
		sprintf_s(buffer, 64, "content\\%s", getFileName());
		// FIXME: assert that file name contains .
		LOG << "Reading simplified json file: " << buffer;
		FlatJSONReader loader;
		if (loader.parse(buffer)) {
			for (int i = 0; i < items.size(); ++i) {
				const SettingsItem& item = items[i];
				if (loader.contains(item.name)) {
					if (item.type == ST_FLOAT) {
						loader.get_float(item.name, item.ptr.fPtr);
					}
					else if (item.type == ST_INT) {
						loader.get(item.name, item.ptr.iPtr);
					}
					else if (item.type == ST_RECT) {
						loader.get(item.name, item.ptr.rPtr);
					}
					else if (item.type == ST_V2_PATH) {
						loader.get(item.name, item.ptr.pPtr);
					}
					else if (item.type == ST_VEC2) {
						loader.get(item.name, item.ptr.v2Ptr);
					}
					else if (item.type == ST_COLOR) {
						loader.get(item.name, item.ptr.cPtr);
					}
				}
			}
		}
		/*
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
		*/
		s.end();
		LOG << "----> elapsed: " << s.elapsed();
		return ret;
	}

	// -------------------------------------------------------
	// export json
	// -------------------------------------------------------
	/*
	bool DynamicGameSettings::saveData(JSONWriter& writer) {
		writer.startCategory("settings");
		for (int i = 0; i < items.size(); ++i) {
			const SettingsItem& item = items[i];
			if (item.type == ST_INT) {
				writer.write(item.name, *item.ptr.iPtr);
			}
			else if (item.type == ST_FLOAT) {
				writer.write(item.name, *item.ptr.fPtr);
			}
			else if (item.type == ST_RECT) {
				writer.write(item.name, *item.ptr.rPtr);
			}
			else if (item.type == ST_V2_PATH) {
				writer.write(item.name, *item.ptr.pPtr);
			}
			else if (item.type == ST_COLOR) {
				writer.write(item.name, *item.ptr.cPtr);
			}
			else if (item.type == ST_V2_PATH) {
				writer.write(item.name, *item.ptr.pPtr);
			}
		}
		writer.endCategory();
		return true;
	}
	*/

}