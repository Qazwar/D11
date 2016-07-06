#pragma once
#include "..\math\vector.h"
#include "..\math\math_types.h"
#include "..\renderer\render_types.h"
#include "..\utils\Color.h"
#include "..\lib\collection_types.h"
#include "..\imgui\IMGUI.h"
#include "..\io\DataFile.h"

namespace ds {

	enum SettingsType {
		ST_FLOAT,
		ST_RECT,
		ST_INT,
		ST_V2_PATH,
		ST_COLOR,
		ST_NONE
	};

	// -------------------------------------------------------
	// settings item
	// -------------------------------------------------------
	struct SettingsItem {
		const char* name;
		IdString hash;
		SettingsType type;
		union {
			int* iPtr;
			float* fPtr;
			v2* v2Ptr;
			v3* v3Ptr;
			Color* cPtr;
			Rect* rPtr;
			Vector2fPath* pPtr;
		} ptr;
	};
	
	// -------------------------------------------------------
	// dynamic settings
	// -------------------------------------------------------
	struct DynamicGameSettings : public DataFile {

		DynamicGameSettings();
		~DynamicGameSettings();

		void add(const char* name, float* value, float defaultValue = 0.0f);
		bool set(const char* name, float value);

		//void addInt(const char* name, int* value, int defaultValue);
		//bool setInt(const char* name, int value);

		//void addRect(const char* name, Rect* value, const Rect& defaultValue);
		//bool setRect(const char* name, const Rect& value);

		//void addColor(const char* name, Color* value, const Color& defaultValue);
		//bool setColor(const char* name, const Color& value);

		//void addPath(const char* name, Vector2fPath* value);
		//bool setPath(const char* name, const Vector2fPath& value);
		
		bool saveData(JSONWriter& writer);
		bool loadData(const JSONReader& loader);

		virtual const char* getFileName() const = 0;

		//int _state;
		//int _offset;
		//Array<SettingsItem> _items;
		//gui::ComponentModel<SettingsItem> _model;

		Array<SettingsItem> items;
	};

}