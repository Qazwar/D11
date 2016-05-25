#pragma once
#include "DynamicSettings.h"
#include "..\utils\Log.h"

namespace ds {

	namespace settings {

		struct SettingsContext {
			Array<CVar> items;
			CharBuffer name_buffer;
			CharBuffer descriptions;
		};

		static SettingsContext* _settingsCtx = 0;

		// ----------------------------------------------------
		// initialize
		// ----------------------------------------------------
		void initialize() {
			assert(_settingsCtx == 0);
			_settingsCtx = new SettingsContext;
			ds::JSONReader reader;
			if (reader.parse("content\\settings.json")) {
				LOG << "loading settings file";
				int mainIndex = reader.find_category("settings");
				if (mainIndex != -1) {
					int children[256];
					int type = 0;
					int num = reader.get_categories(children, 256, mainIndex);
					for (int i = 0; i < num; ++i) {
						if (reader.matches(children[i], "var")) {
							const char* name = reader.get_string(children[i], "name");
							reader.get_int(children[i], "type", &type);
							if (type == 1) {
								float v = 0.0f;
								reader.get(children[i], "value", &v);
								addFloat(name, "Test", v);
							}
						}
					}
				}
			}
			else {
				LOG << "No settings file found";
			}
		}

		// ----------------------------------------------------
		// shutdown
		// ----------------------------------------------------
		void shutdown() {
			ds::JSONWriter writer;
			if (writer.open("content\\settings.json")) {
				writer.startCategory("settings");
				for (int i = 0; i < _settingsCtx->items.size(); ++i) {
					const CVar& c = _settingsCtx->items[i];
					writer.startCategory("var");
					if (c.type == ST_FLOAT) {
						const char* name = _settingsCtx->name_buffer.data + c.nameIndex;
						writer.write("name", name);
						writer.write("type", 1);
						writer.write("value", *c.ptr.fPtr);
					}
					writer.endCategory();
				}
				writer.endCategory();
			}
			// save file
			for (int i = 0; i < _settingsCtx->items.size(); ++i) {
				const CVar& c = _settingsCtx->items[i];
				if (c.type == ST_FLOAT) {
					delete c.ptr.fPtr;
				}
			}
			if (_settingsCtx != 0) {
				delete _settingsCtx;
			}
		}

		// ----------------------------------------------------
		// find index of cvar
		// ----------------------------------------------------
		int find(const char* name, SettingsType type) {
			IdString hash = string::murmur_hash(name);
			for (int i = 0; i < _settingsCtx->items.size(); ++i) {
				const CVar& c = _settingsCtx->items[i];
				if (c.hash == hash && c.type == type) {
					return i;
				}
			}
			return -1;
		}

		// ----------------------------------------------------
		// get cvar by name
		// ----------------------------------------------------
		const CVar& get(const char* name) {
			IdString hash = string::murmur_hash(name);
			for (int i = 0; i < _settingsCtx->items.size(); ++i) {
				const CVar& c = _settingsCtx->items[i];
				if (c.hash == hash) {
					return c;
				}
			}
			return INVALID_CVAR;
		}

		// ----------------------------------------------------
		// add float
		// ----------------------------------------------------
		float* addFloat(const char* name, const char* description, float defaultValue) {
			int index = find(name, SettingsType::ST_FLOAT);
			if (index != -1) {
				const CVar& c = _settingsCtx->items[index];
				return c.ptr.fPtr;
			}
			CVar item;
			item.nameIndex = _settingsCtx->name_buffer.size;
			_settingsCtx->name_buffer.append(name);
			item.type = ST_FLOAT;
			item.ptr.fPtr = new float;
			item.hash = string::murmur_hash(name);
			*item.ptr.fPtr = defaultValue;
			_settingsCtx->items.push_back(item);
			return item.ptr.fPtr;
		}
	}

}