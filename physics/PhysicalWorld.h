#pragma once
#include "..\sprites\SpriteArray.h"
#include "ColliderArray.h"
#include <map>
#include "..\math\Bitset.h"
#include "..\lib\collection_types.h"

namespace ds {

	struct IgnoredCollision {

		int firstType;
		int secondType;

		bool matches(int first, int second) {
			if (first == firstType && second == secondType) {
				return true;
			}
			if (second == firstType && first == secondType) {
				return true;
			}
			return false;
		}
	};

	typedef Array<IgnoredCollision> IgnoredCollisions;

	struct PotentialCollider {
		SID first;
		SID second;
		int firstIndex;
		int secondIndex;
	};

	class PotentialCollisionDetector {

	public:
		PotentialCollisionDetector() {}
		virtual ~PotentialCollisionDetector() {}
		void reset();
		virtual void add(int firstIndex, SID first, int firstType, int secondIndex, SID second, int secondType) = 0;
		int size() const;
		void ignore(int firstType, int secondType);
		const PotentialCollider& get(int index) const;
	protected:
		bool contains(SID first, SID second) const;
		bool shouldBeIgnored(int firstType, int secondType);
		//Array<PotentialCollider> _colliders;
		PotentialCollider _colliders[4096];
		int num;
		IgnoredCollisions _ignored;
	};

	class BruteForceCollisionDetector : public PotentialCollisionDetector {

	public:
		BruteForceCollisionDetector() {}
		virtual ~BruteForceCollisionDetector() {}
		void add(int firstIndex,SID first, int firstType,int secondIndex, SID second, int secondType);
	};

	class SpatialGridCollisionDetector : public PotentialCollisionDetector {

	public:
		SpatialGridCollisionDetector(const Point& gridSize) : PotentialCollisionDetector() , _gridSize(gridSize) {}
		virtual ~SpatialGridCollisionDetector() {}
		void add(int firstIndex, SID first, int firstType, int secondIndex, SID second, int secondType);
	private:
		Point _gridSize;
	};

	class PhysicalWorld {

	public:
		PhysicalWorld();
		~PhysicalWorld();
		void ignore(int firstType, int secondType);
		void ignoreLayer(int layer);
		bool hasCollisions() {
			return _collisions.size() > 0;
		}
		int getNumCollisions() {
			return _collisions.size();
		}
		const Collision& getCollision(int idx) const {
			return _collisions[idx];
		}
		void tick(SpriteArray* sprites,float dt);
		void drawColliders(const Texture& texture);
	private:
		bool intersects(SpriteArray* sprites,int firstIndex, int secondIndex);
		void checkCollisions(SpriteArray* sprites);
		bool containsCollision(const Collision& c);
		Array<Collision> _collisions;
		IgnoredCollisions m_Ignored;
		Bits _ignoredLayers;
		PotentialCollisionDetector* _potentialColliders;
	};

}

