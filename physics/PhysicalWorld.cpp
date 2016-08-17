#include "PhysicalWorld.h"
#include "..\utils\Log.h"
#include "..\utils\Profiler.h"

namespace ds {

	// --------------------------------------------------------------------------
	// PotentialColliders
	// --------------------------------------------------------------------------
	void PotentialCollisionDetector::reset() {
		//_colliders.clear();
		num = 0;
	}

	int PotentialCollisionDetector::size() const {
		//return _colliders.size();
		return num;
	}

	const PotentialCollider& PotentialCollisionDetector::get(int index) const {
		return _colliders[index];
	}

	bool PotentialCollisionDetector::contains(SID first, SID second) const {
		//for (int i = 0; i < _colliders.size(); ++i) {
		for (int i = 0; i < num; ++i) {
			if (_colliders[i].first == first && _colliders[i].second == second) {
				return true;
			}
			if (_colliders[i].first == second && _colliders[i].second == first) {
				return true;
			}
		}
		return false;
	}

	bool PotentialCollisionDetector::shouldBeIgnored(int firstType, int secondType) {
		for (size_t i = 0; i < _ignored.size(); ++i) {
			IgnoredCollision& ic = _ignored[i];
			if (ic.matches(firstType, secondType)) {
				return true;
			}
		}
		return false;
	}

	void PotentialCollisionDetector::ignore(int firstType, int secondType) {
		if (!shouldBeIgnored(firstType, secondType)) {
			IgnoredCollision c;
			c.firstType = firstType;
			c.secondType = secondType;
			_ignored.push_back(c);
		}
	}

	void BruteForceCollisionDetector::add(int firstIndex, SID first, int firstType, int secondIndex, SID second, int secondType) {
		if (!shouldBeIgnored(firstType, secondType)) {
			//if (!contains(first, second)) {
				//PotentialCollider pc;
				PotentialCollider& pc = _colliders[num++];
				pc.firstIndex = firstIndex;
				pc.first = first;
				pc.secondIndex = secondIndex;
				pc.second = second;
				//_colliders.push_back(pc);
			//}
		}
	}

	void SpatialGridCollisionDetector::add(int firstIndex, SID first, int firstType, int secondIndex, SID second, int secondType) {
		if (!shouldBeIgnored(firstType, secondType)) {



			if (!contains(first, second)) {
				//PotentialCollider pc;
				PotentialCollider& pc = _colliders[num++];
				pc.firstIndex = firstIndex;
				pc.first = first;
				pc.secondIndex = secondIndex;
				pc.second = second;
				//_colliders.push_back(pc);
			}
		}
	}
	// --------------------------------------------------------------------------
	// PhysicalWorld
	// --------------------------------------------------------------------------
	PhysicalWorld::PhysicalWorld() {
		_potentialColliders = new BruteForceCollisionDetector;
	}


	PhysicalWorld::~PhysicalWorld() {
		delete _potentialColliders;
		//if (m_ColliderData.buffer != 0) {
			//delete[] m_ColliderData.buffer;
			//DEALLOC(m_ColliderData.buffer);
		//}
	}

	// --------------------------------------------------------------------------
	// set sprite array pointer
	// --------------------------------------------------------------------------
	//void PhysicalWorld::setDataPtr(SpriteArray* sprites) {
		//m_Sprites = sprites;
	//}

	// --------------------------------------------------------------------------
	// remove by SID
	// --------------------------------------------------------------------------
	/*
	void PhysicalWorld::remove(SID id) {
		ZoneTracker z("PhysicalWorld::remove");
		for (int i = 0; i < m_ColliderData.num; ++i) {
			if (m_ColliderData.sids[i] == id) {
				m_ColliderData.remove(m_ColliderData.ids[i]);
			}
		}
	}
	*/
	// --------------------------------------------------------------------------
	// tick
	// --------------------------------------------------------------------------
	void PhysicalWorld::tick(SpriteArray* sprites,float dt) {
		ZoneTracker z("PhysicalWorld::tick");
		_collisions.clear();
		checkCollisions(sprites);
		// update previous positions
		for (int i = 0; i < sprites->num; ++i) {
			if (sprites->shapeTypes[i] != SST_NONE) {
				sprites->previous[i] = sprites->positions[i];
			}
		}
	}

	// --------------------------------------------------------------------------
	// check collisions
	// --------------------------------------------------------------------------
	void PhysicalWorld::checkCollisions(SpriteArray* sprites) {
		ZoneTracker z("PhysicalWorld::checkCollisions");
		{
			ZoneTracker z1("PhysicalWorld::potentials");
			_potentialColliders->reset();
			for (int i = 0; i < sprites->num; ++i) {
				if (sprites->shapeTypes[i] != SST_NONE) {
					if (!_ignoredLayers.isSet(sprites->layers[i])) {
						for (int j = i + 1; j < sprites->num; ++j) {
							if (sprites->shapeTypes[j] != SST_NONE) {
								if (!_ignoredLayers.isSet(sprites->layers[j])) {
									if (i != j) {
										_potentialColliders->add(i, sprites->ids[i], sprites->types[i], j, sprites->ids[j], sprites->types[j]);
									}
								}
							}
						}
					}
				}
			}
		}
		{
			ZoneTracker z2("PhysicalWorld::check");
			for (int x = 0; x < _potentialColliders->size(); ++x){
				const PotentialCollider& pc = _potentialColliders->get(x);
				if (intersects(sprites,pc.firstIndex, pc.secondIndex)) {
					Collision c;
					c.firstPos = sprites->positions[pc.firstIndex];
					c.firstSID = sprites->ids[pc.firstIndex];
					c.firstType = sprites->types[pc.firstIndex];
					c.secondPos = sprites->positions[pc.secondIndex];
					c.secondSID = sprites->ids[pc.secondIndex];
					c.secondType = sprites->types[pc.secondIndex];
					if (!containsCollision(c)) {
						_collisions.push_back(c);
					}
				}
			}
		}
	}

	bool PhysicalWorld::containsCollision(const Collision& c) {
		for (uint32_t i = 0; i < _collisions.size(); ++i) {
			const Collision& other = _collisions[i];
			if (other.firstSID == c.firstSID && other.secondSID == c.secondSID) {
				return true;
			}
			if (other.firstSID == c.secondSID && other.secondSID == c.firstSID) {
				return true;
			}
		}
		return false;
	}

	// --------------------------------------------------------------------------
	// ignore layer
	// --------------------------------------------------------------------------
	void PhysicalWorld::ignoreLayer(int layer) {
		assert(layer >= 0 && layer < 32);
		LOG << "ignoring layer: " << layer;
		_ignoredLayers.set(layer);
	}

	// --------------------------------------------------------------------------
	// intersects
	// --------------------------------------------------------------------------
	bool PhysicalWorld::intersects(SpriteArray* sprites,int firstIndex, int secondIndex) {
		SpriteShapeType firstShape = sprites->shapeTypes[firstIndex];
		SpriteShapeType secondShape = sprites->shapeTypes[secondIndex];
		const v2& fp = sprites->positions[firstIndex];
		const v2& fpp = sprites->previous[firstIndex];
		const v2& fe = sprites->extents[firstIndex];
		const v2& sp = sprites->positions[secondIndex];
		const v2& spp = sprites->previous[secondIndex];
		const v2& se = sprites->extents[secondIndex];
		if (firstShape == SST_CIRCLE && secondShape == SST_CIRCLE)  {
			// FIXME: calculate radius based on scale of sprite
			float r1 = fe.x * 0.5f;
			float r2 = se.x * 0.5f;
			float u0, u1;
			if( physics::testCircleSweepIntersection(r1, fpp, fp, r2, spp, sp, &u0, &u1) ) {
			//if (physics::testCircleIntersection(fp, r1, sp, r2)) {
				return true;
			}
		}
		else if (firstShape == SST_BOX && secondShape == SST_BOX)  {
			if (physics::testBoxIntersection(fp,fe,sp,se)) {
				return true;
			}
		}
		return false;
	}
	
	// --------------------------------------------------------------------------
	// ignore
	// --------------------------------------------------------------------------
	void PhysicalWorld::ignore(int firstType, int secondType) {
		_potentialColliders->ignore(firstType, secondType);
	}

	// --------------------------------------------------------------------------
	// draw colliders
	// --------------------------------------------------------------------------
	void PhysicalWorld::drawColliders(const Texture& texture) {
		/*
		for (int i = 0; i < m_ColliderData.num; ++i) {
			v2& e = m_ColliderData.extents[i];
			const v2& scale = m_Sprites->getScale(m_ColliderData.sids[i]);
			float sx = e.x / texture.dim.x * scale.x;
			float sy = e.y / texture.dim.y * scale.y;
			sprites::draw(m_ColliderData.positions[i], texture, 0.0f, sx, sy);
		}
		*/
	}

}