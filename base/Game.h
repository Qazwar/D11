#pragma once
#include "..\lib\collection_types.h"
#include "..\scene\Scene.h"

namespace ds {

	class Game {

	public:
		Game();
		~Game();
		void render();
		void update(float dt);
		void addScene(const char* name, Scene* scene);
		Scene* getScene(const char* name) const;
		void activateScene(const char* name);
		void deactivateScene(const char* name);
	private:
		int findSceneIndex(const char* name) const;
		Array<IdString> _names;
		Array<Scene*> _scenes;
	};

}