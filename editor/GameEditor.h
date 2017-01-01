#pragma once
#include "EditorPlugin.h"
#include <core\lib\collection_types.h>
#include <Windows.h>

namespace ds {

	struct EditorShortcut {
		const char* label;
		unsigned int key;
		int index;
	};

	class GameEditor {

	public:
		GameEditor();
		~GameEditor();
		void tick(float dt);
		void render();
		bool onKey(unsigned int key);
		void addPlugin(const char* label,unsigned int key, EditorPlugin* plugin);
		void showDialog();
		void activate(const char* name);
		void deactivate(const char* name);
		bool isActive(const char* name) const;
		void print();
		void toggle();
	private:
		Array<EditorPlugin*> _plugins;
		Array<EditorShortcut> _shortcuts;
		bool _active;
	};

}
