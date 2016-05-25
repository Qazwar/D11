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

	struct CVar {
		int nameIndex;
		SettingsType type;
		IdString hash;
		union {
			int* iPtr;
			float* fPtr;
			v2* v2Ptr;
			v3* v3Ptr;
			Color* cPtr;
			Rect* rPtr;
		} ptr;

		CVar() : nameIndex(-1), type(ST_NONE), hash(0) {}
		CVar(int ni, SettingsType t, IdString h) : nameIndex(ni), type(type), hash(h) {}

	};

	const CVar INVALID_CVAR = CVar(-1, ST_NONE, 0);

	namespace settings {

		void initialize();

		void shutdown();

		float* addFloat(const char* name, const char* description, float defaultValue = 0.0f);

		const CVar& get(const char* name);
	}

}