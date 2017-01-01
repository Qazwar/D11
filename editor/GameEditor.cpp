#include "GameEditor.h"
#include <Windows.h>
#include <core\log\Log.h>
#include "..\renderer\graphics.h"
#include "..\imgui\IMGUI.h"

namespace ds {

	GameEditor::GameEditor() : _active(false) {
	}

	GameEditor::~GameEditor() {
	}

	void GameEditor::render() {
		for (uint32_t i = 0; i < _plugins.size(); ++i) {
			if (_plugins[i]->isActive()) {
				_plugins[i]->showDialog();
			}
		}
	}

	bool GameEditor::onKey(unsigned int key) {
		for (uint32_t i = 0; i < _shortcuts.size(); ++i) {
			const EditorShortcut& esc = _shortcuts[i];
			if (esc.key == key) {
				EditorPlugin* plugin = _plugins[esc.index];
				if (plugin->isActive()) {
					plugin->deactivate();
				}
				else {
					plugin->activate();
				}
			}
		}
		return false;
	}

	void GameEditor::addPlugin(const char* label,unsigned int key, EditorPlugin * plugin) {
		int sz = _plugins.size();
		_plugins.push_back(plugin);
		EditorShortcut esc;
		esc.label = label;
		esc.index = sz;
		esc.key = key;
		_shortcuts.push_back(esc);
	}

	void GameEditor::showDialog() {
		if (_active) {
			v2 p(10, graphics::getScreenHeight() - 10.0f);
			gui::start(1, &p);
			int state = 1;
			gui::begin("Editor", &state);
			for (uint32_t i = 0; i < _plugins.size(); ++i) {
				EditorPlugin* plugin = _plugins[i];
				if (gui::Button(plugin->getName())) {
					if (plugin->isActive()) {
						plugin->deactivate();
					}
					else {
						plugin->activate();
					}
					_active = false;
				}
			}
			gui::end();
		}
	}

	void GameEditor::toggle() {
		_active = !_active;
	}

	void GameEditor::print() {
		for (uint32_t i = 0; i < _shortcuts.size(); ++i) {
			LOG << _shortcuts[i].label << " = " << _plugins[_shortcuts[i].index]->getName();
		}
	}

}