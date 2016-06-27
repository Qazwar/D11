#pragma once
#include "..\lib\DataArray.h"
#include <Vector.h>
#include "..\Common.h"
#include "MeshBuffer.h"
#include "Camera.h"

namespace ds {

	enum DrawMode {
		STATIC,
		TRANSFORM,
		IMMEDIATE
	};

	struct StaticMesh {
		ID id;
		uint32_t index;
		uint32_t size;
		AABBox boundingBox;
	};

	struct Entity {
		ID id;
		Mesh* mesh;
		v3 position;
		v3 scale;
		v3 rotation;
		float timer;
		bool active;
		int type;
		mat4 world;
		ID parent;
		Color color;
		int value;
		DrawMode mode;
		int staticIndex;
		RID material;
	};

	typedef DataArray<Entity> EntityList;

	class Scene {

	public:
		Scene(const SceneDescriptor& descriptor);
		~Scene();
		ID add(const char* meshName, const v3& position, RID material, DrawMode mode = IMMEDIATE);
		ID add(Mesh* mesh, const v3& position, RID material, DrawMode mode = IMMEDIATE);
		ID addStatic(Mesh* mesh, const v3& position, RID material);
		void attach(ID child, ID parent);
		Entity& get(ID id);
		const Entity& get(ID id) const;
		void remove(ID id);
		void draw();
		int find(int type, ID* ids, int max);
		void transform();
		ID intersects(const Ray& ray);
		uint32_t numEntities() const {
			return _entities.numObjects;
		}
	private:
		RID _currentMaterial;
		EntityList _entities;
		MeshBuffer* _meshBuffer;
		Camera* _camera;
		bool _depthEnabled;
		Array<PNTCVertex> _staticVertices;
		Array<StaticMesh> _staticMeshes;
	};

}
