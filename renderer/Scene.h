#pragma once
#include "..\lib\DataArray.h"
#include <Vector.h>
#include "..\Common.h"
#include "MeshBuffer.h"

namespace ds {

	struct Entity {
		ID id;
		Mesh* mesh;
		v3 position;
		v3 scale;
		v3 rotation;
		float timer;
		bool active;
		int type;
	};

	typedef DataArray<Entity> EntityList;

	class Scene {

	public:
		Scene(const char* meshBufferName);
		~Scene();
		ID add(const char* meshName, const v3& position);
		Entity& get(ID id);
		const Entity& get(ID id) const;
		void remove(ID id);
		void draw();
		int find(int type, ID* ids, int max);
	private:
		EntityList _entities;
		MeshBuffer* _meshBuffer;
	};

}
