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

	// ----------------------------------------
	// Basic scene
	// ----------------------------------------
	class Scene {

	public:
		Scene(const SceneDescriptor& descriptor);
		~Scene();
		ID add(const char* meshName, const v3& position, RID material, DrawMode mode = IMMEDIATE);
		ID add(Mesh* mesh, const v3& position, RID material, DrawMode mode = IMMEDIATE);
		ID addStatic(Mesh* mesh, const v3& position, RID material);
		void attach(ID child, ID parent);
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

		// actions
		void scaleTo(ID sid, const v3& startScale, const v3& endScale, float ttl, int mode = 0, const tweening::TweeningType& tweeningType = &tweening::linear);
		void moveTo(ID sid, const v3& startPos, const v3& endPos, float ttl, int mode = 0, const tweening::TweeningType& tweeningType = &tweening::linear);
		void rotateTo(ID sid, const v3& startRotation, const v3& endRotation, float ttl, int mode = 0, const tweening::TweeningType& tweeningType = &tweening::linear);

		bool hasEvents() const {
			return _eventBuffer.events.size() > 0;
		}
		uint32_t numEvents() const {
			return _eventBuffer.events.size();
		}

		const ActionEvent& getEvent(uint32_t idx) {
			return _eventBuffer.events[idx];
		}


		void save(const ReportWriter& writer);
		const bool isActive() const {
			return _active;
		}
		void setActive(bool active) {
			_active = active;
		}
	private:
		bool _active;
		void updateWorld(int idx);

		SceneDescriptor _descriptor;
		RID _currentMaterial;
		EntityArray _data;
		MeshBuffer* _meshBuffer;
		Camera* _camera;
		Array<PNTCVertex> _staticVertices;
		Array<StaticMesh> _staticMeshes;
		AbstractAction* _actions[MAX_ACTIONS];
		ActionEventBuffer _eventBuffer;
		bool _depthEnabled;
	};

	// ----------------------------------------
	// 2D scene
	// ----------------------------------------
	class Scene2D : public Scene {

	public:
		Scene2D(const SceneDescriptor& descriptor) : Scene(descriptor) {}
		~Scene2D() {}
	private:
		OrthoCamera* _camera;
	};

	// ----------------------------------------
	// 3D scene
	// ----------------------------------------
	class Scene3D : public Scene {

	public:
		Scene3D(const SceneDescriptor& descriptor) : Scene(descriptor) {}
		~Scene3D() {}
	private:
		FPSCamera* _camera;
	};
}
