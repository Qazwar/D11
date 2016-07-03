#pragma once
#include "..\lib\DataArray.h"
#include <Vector.h>
#include "..\Common.h"
#include "..\renderer\MeshBuffer.h"
#include "..\renderer\Camera.h"
#include "EntityArray.h"
#include "ActionEventBuffer.h"
#include "..\math\tweening.h"
#include "actions\AbstractAction.h"

namespace ds {

	const int MAX_ACTIONS = 32;

	struct StaticMesh {
		ID id;
		uint32_t index;
		uint32_t size;
		AABBox boundingBox;
	};

	//typedef DataArray<Entity> EntityList;

	class Scene {

	public:
		Scene(const SceneDescriptor& descriptor);
		~Scene();
		ID add(const char* meshName, const v3& position, RID material, DrawMode mode = IMMEDIATE);
		ID add(Mesh* mesh, const v3& position, RID material, DrawMode mode = IMMEDIATE);
		ID addStatic(Mesh* mesh, const v3& position, RID material);
		void attach(ID child, ID parent);
		//Entity& get(ID id);
		//const Entity& get(ID id) const;
		void remove(ID id);
		void draw();
		int find(int type, ID* ids, int max);
		ID intersects(const Ray& ray);
		uint32_t numEntities() const {
			return _data.num;
		}
		void tick(float dt);
		void rotate(ID id, const v3& r);
		void activate(ID id);
		void deactivate(ID id);
		void setColor(ID id, const Color& clr);
		void clear();
		bool isActive(ID id) const;
		void setPosition(ID id, const v3& p);

		void scaleTo(ID sid, const v3& startScale, const v3& endScale, float ttl, int mode = 0, const tweening::TweeningType& tweeningType = &tweening::linear);
		void moveTo(ID sid, const v3& startPos, const v3& endPos, float ttl, int mode = 0, const tweening::TweeningType& tweeningType = &tweening::linear);

		void save(const ReportWriter& writer);
	private:
		void updateWorld(int idx);

		SceneDescriptor _descriptor;
		RID _currentMaterial;
		EntityArray _data;
		//EntityList _entities;
		MeshBuffer* _meshBuffer;
		Camera* _camera;
		bool _depthEnabled;
		Array<PNTCVertex> _staticVertices;
		Array<StaticMesh> _staticMeshes;
		AbstractAction* _actions[MAX_ACTIONS];
		ActionEventBuffer _eventBuffer;
	};

}
