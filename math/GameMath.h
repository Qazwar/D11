#pragma once
#include "..\Common.h"
#include "..\utils\Color.h"
#include "math_types.h"
#include "..\renderer\render_types.h"
#include "..\physics\physic_types.h"
#include <Vector.h>

Vector2f rotate(float angle,float vel);

namespace ds {


	namespace math {

		float norm(float t,float max);
		bool addClamped(float *value,float add,float max,float newValue = 0.0f);
		float saturate(float x);
		float smoothstep(float min,float max,float x);
		float smoothstep(float x);
		void radial_velocity(float angle, float velocity, v2* vel);
		Vector2f getRadialVelocity(float angle,float vel);
		Vector2f getTargetVelocity(const Vector2f& targetPosition,const Vector2f& currentPosition,float* angle,float vel);
		float getAngle(float x1,float y1,float x2,float y2);
		float getAngle(const Vector2f& v1,const Vector2f& v2);
		float getTargetAngle(const Vector2f& v1,const Vector2f& v2);
		Vector2f getDistantPosition(float x,float y,float angle,float radius);
		Vector2f getDistantPosition(const Vector2f& initialPosition,float angle,float radius);
		float impulse(float k,float x );
		float cubicPulse( float c, float w, float x );
		float almostIdentity( float x, float m, float n );
		void transformMousePosToWorld(Vector3f* vPickRayDir,Vector3f* vPickRayOrig);
		void follow(const Vector2f& targetPos,Vector2f& newPos,float* angle,float distance,float add);
		void followRelative(const Vector2f& targetPos,Vector2f& newPos,float* angle,float distance,float percentage);
		void move_towards(const Vector2f& targetPos, Vector2f& newPos, float* angle, float velocity, float dt);
		void clipToRect(Vector2f& newPos,const Rect& rect);
		bool isOutside(Vector2f& pos,const Rect& rect);
		bool isInside(Vector2f& pos, const Rect& rect);
		float reflect(float angle);
		v2 reflect(const v2& v, const v2& n);
		//void interpolateColor(const Color& firstCol,const Color& secondColor,float t,Color* outColor);
		bool outside(float value,float min,float max);
		// -------------------------------------------------------
		// 
		// -------------------------------------------------------
		bool checkLineCircle(const Vector2f& center, float radius,const Vector2f& lineFrom,const Vector2f& lineTo);
		

		// -------------------------------------------------------
		// Check if circle and box overlap
		// -------------------------------------------------------
		bool checkCircleBoxIntersection(const v2& circlePos, float radius, const Rect& rectangle);

		bool checkCircleBoxIntersection(const v2& circlePos, float radius, const v2& pos,const v2& dim);

		bool checkCircleBoxIntersection(const v2& circlePos, float radius, const AABBox& aabBox);
		// -------------------------------------------------------
		// Check if two circles overlap
		// -------------------------------------------------------
		bool checkCircleIntersection(const Vector2f& p1,float r1,const Vector2f& p2,float r2);
		// ------------------------------------------------------------------------------------------
		// Check if two circles overlap also calculating penetration distance and penetration vector
		// ------------------------------------------------------------------------------------------
		bool checkCircleIntersection(const Vector2f& p1,float r1,const Vector2f& p2,float r2,float* dist,Vector2f& penVec);

		Vector2f getShiftVector(const Vector2f& p1,float r1,const Vector2f& p2,float r2);

		bool solveQuadraticFormula(const float a,const float b,const float c,float* r1,float* r2);

		bool circleSweepTest(const Vector2f& a0,const Vector2f& a1,float ra,const Vector2f& b0,const Vector2f& b1,float rb,float* u0,float* u1);	

		uint32_t nextPowerOf2(uint32_t v);

	}
}
