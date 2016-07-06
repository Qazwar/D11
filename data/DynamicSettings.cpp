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

	// -------------------------------------------------------
	// export json
	// -------------------------------------------------------
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

	// -------------------------------------------------------
	// import json
	// -------------------------------------------------------
	bool DynamicGameSettings::loadData(const JSONReader& loader) {
		StopWatch sw("DNG::loadData");
		int c = loader.find_category("settings");
		if (c != -1) {
			for (int i = 0; i < items.size(); ++i) {
				const SettingsItem& item = items[i];
				if (loader.contains_property(c, item.name)) {
					if (item.type == ST_FLOAT) {
						loader.get_float(c, item.name, item.ptr.fPtr);
					}
					else if (item.type == ST_INT) {
						loader.get_int(c, item.name, item.ptr.iPtr);
					}
					else if (item.type == ST_RECT) {
						loader.get(c, item.name, item.ptr.rPtr);
					}
					else if (item.type == ST_V2_PATH) {
						loader.get_vec2_path(c, item.name, item.ptr.pPtr);
					}
					else if (item.type == ST_COLOR) {
						loader.get_color(c, item.name, item.ptr.cPtr);
					}
				}
			}
		}
		return true;
	}


}