#pragma once
#include "core\lib\collection_types.h"
#include "..\scene\Scene.h"
#include "core\string\StaticHash.h"

namespace ds {

	class Game {

	public:
		Game();
		~Game();
		void render();
		void update(float dt);
		Scene2D* create2DScene(const char* name);
		void addScene(const char* name, Scene* scene);
		Scene* getScene(const char* name) const;
		void activateScene(const char* name);
		void deactivateScene(const char* name);

		void save(const ReportWriter& writer);
	private:
		int findSceneIndex(const char* name) const;
		Array<StaticHash> _names;
		Array<Scene*> _scenes;
	};

}