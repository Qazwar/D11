#pragma once
#include "DynamicSettings.h"
#include "..\utils\Log.h"

namespace ds {

	

	DynamicGameSettings::DynamicGameSettings(const char* fileName) : FlatJSONAssetFile(fileName) {

	}

	DynamicGameSettings::~DynamicGameSettings() {

	}

	void DynamicGameSettings::add(const char* name, float* value, float defaultValue) {
		*value = defaultValue;
		SettingsItem item;
		item.name = name;
		item.hash = StaticHash(name);
		item.type = ST_FLOAT;
		item.ptr.fPtr = value;
		items.push_back(item);
	}

	void DynamicGameSettings::add(const char* name, int* value, int defaultValue) {
		*value = defaultValue;
		SettingsItem item;
		item.name = name;
		item.hash = StaticHash(name);
		item.type = ST_INT;
		item.ptr.iPtr = value;
		items.push_back(item);
	}

	void DynamicGameSettings::add(const char* name, v2* value, const v2& defaultValue) {
		*value = defaultValue;
		SettingsItem item;
		item.name = name;
		item.hash = StaticHash(name);
		item.type = ST_VEC2;
		item.ptr.v2Ptr = value;
		items.push_back(item);
	}

	// -----------------------------------------------
	// load JSON
	// -----------------------------------------------
	bool DynamicGameSettings::reloadData(const FlatJSONReader& loader) {
		return loadData(loader);
	}

	bool DynamicGameSettings::loadData(const FlatJSONReader& loader) {
		StopWatch s;
		s.start();
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
		s.end();
		LOG << "----> elapsed: " << s.elapsed();
		return true;
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