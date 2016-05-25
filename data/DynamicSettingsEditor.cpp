#include "DynamicSettingsEditor.h"

namespace ds {

DynamicSettingsEditor::DynamicSettingsEditor(DynamicGameSettings* settings) 
	: AssetEditor("DynamicSettingsEditor",settings) , _settings(settings) , _state(1) , _position(10,750) , _shortcut("SET") {
}

DynamicSettingsEditor::DynamicSettingsEditor(const char* shortcut, DynamicGameSettings* settings)
	: AssetEditor("DynamicSettingsEditor", settings), _settings(settings), _state(1), _position(10, 750), _shortcut(shortcut) {
}

DynamicSettingsEditor::~DynamicSettingsEditor() {
}

void DynamicSettingsEditor::showDialog() {
	gui::start(20, &_position);
	if (gui::begin("Settings Dialog", &_state)) {
		for (int i = 0; i < _settings->_items.size(); ++i) {
			const SettingsItem& element = _settings->_items[i];
			if (element.type == ST_FLOAT) {
				gui::InputFloat(element.name, element.ptr.fPtr);
			}
			else if (element.type == ST_INT) {
				gui::InputInt(element.name, element.ptr.iPtr);
			}
			else if (element.type == ST_RECT) {
				gui::InputRect(element.name, element.ptr.rPtr);
			}
			else if (element.type == ST_COLOR) {
				gui::InputColor(element.name, element.ptr.cPtr);
			}
		}
		gui::beginGroup();
		if (gui::Button("Save")) {
			_settings->save();
		}
		if (gui::Button("Load")) {
			_settings->load();
		}
		gui::endGroup();
	}
	gui::end();
}

void DynamicSettingsEditor::init() {
}

}