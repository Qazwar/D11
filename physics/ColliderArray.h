#pragma once
#include <Vector.h>
#include "..\renderer\render_types.h"
#include "..\sprites\Sprite.h"
#include "physic_types.h"

namespace ds {

	struct Collision {

		v2 firstPos;
		v2 secondPos;
		SID firstSID;
		SID secondSID;
		int firstType;
		int secondType;
		v2 norm;
		float distance;

		const bool containsType(int type) const {
			if ( firstType == type ) {
				return true;
			}
			if ( secondType == type ) {
				return true;
			}
			return false;
		}

		SID getSIDByType(int type) const {
			if (firstType == type) {
				return firstSID;
			}
			return secondSID;
		}

		const v2& getPositionByType(int type) const {
			if (type == firstType) {
				return firstPos;
			}
			return secondPos;
		}

	};

	namespace physics {

		void debug(const Collision& c);

		// ------------------------------------------------------------------
		// test if a line from A to B intersects with a line from C to D
		// ------------------------------------------------------------------
		bool testLineIntersections(const v2& a, const v2& b, const v2& c, const v2& d);

		int testLineBox(const v2& lineStart, const v2& boxCenter, const v2& dim);

		bool isInsideBox(const Vector2f& ray,const Vector2f& p,const Vector2f& dim);

		bool testCircleSweepIntersection(float ra, const Vector2f& A0, const Vector2f& A1, float rb, const Vector2f& B0, const Vector2f&	B1, float* u0, float* u1);

		bool testCircleIntersection(const Vector2f& p1, float r1, const Vector2f& p2, float r2);

		bool testBoxIntersection(const Vector2f& p1,const Vector2f& e1,const Vector2f& p2,const Vector2f& e2);

		bool testBoxIntersection(const Vector3f& p1, const Vector3f& e1, const Vector3f& p2, const Vector3f& e2);

		bool sweepTest(const AABBox& b1, const Vector3f& prev1, const AABBox& b2, const Vector3f& prev2, float* u0, float *u1);

		bool testStaticAABBAABB(AABBox a, AABBox b, Vector3f* normal, float* penetration);
		
	}

}
