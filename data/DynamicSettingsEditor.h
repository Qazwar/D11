#pragma once
#include "..\editor\AssetEditor.h"
#include "DynamicSettings.h"

namespace ds {

	class DynamicSettingsEditor : public AssetEditor {

	public:
		DynamicSettingsEditor(DynamicGameSettings* settings);
		DynamicSettingsEditor(const char* shortcut, DynamicGameSettings* settings);
		~DynamicSettingsEditor();

		void showDialog();

		void init();

		const char* getShortcut() const {
			return _shortcut;
		}

	private:
		const char* _shortcut;
		DynamicGameSettings* _settings;
		v2 _position;
		int _state;
	};

}

