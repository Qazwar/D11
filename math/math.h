#pragma once

#include "..\renderer\render_types.h"

const float PI = 3.1415926535897932384626433832795f;
const float TWO_PI = 2.0f * PI;
const float HALF_PI = 0.5f * PI;
const float TWO_PI_INV = 1.0f / TWO_PI;

namespace math {

	float hill(float x);

	float fastSin(float x);

	float fastCos(float x);

	ds::Texture buildTexture(float top, float left, float width, float height, float textureWidth = 1024.0f, float textureHeight = 1024.0f);

	ds::Texture buildTexture(const ds::Rect& r, float textureWidth = 1024.0f, float textureHeight = 1024.0f);

	void srt(const v2& v, const v2& u, const v2& scale, float rotation, v2* ret);

	v2 srt(const v2& v, const v2& u, float scaleX, float scaleY, float rotation);

	Vector3f srt(const Vector3f& v, const Vector3f& u, float scaleX, float scaleY, float rotation);

	void addRadial(v2& v, float radius, float angle);

	void addRadial(Vector3f& v, float radius, float beta, float phi);

	v2 calculateRadial(const v2& v, float radius, float angle);

	void rotate(v2& v, float angle);

	float getAngle(const v2& v1, const v2& v2);

	float calculateRotation(const v2& v);

	v2 getRadialVelocity(float angle, float velocity);

	void clamp(v2& v, const v2& min, const v2& max);

	float clamp(float value, float min = 0.0f, float max = 1.0f);

	float norm(float t, float max);

	// -------------------------------------------------------
	// Get random float between min and max
	// -------------------------------------------------------
	float random(float min, float max);
	// -------------------------------------------------------
	// Get random v2 between min and max
	// -------------------------------------------------------
	v2 random(const v2& min, const v2& max);
	// -------------------------------------------------------
	// Get random int between min and max
	// -------------------------------------------------------
	int random(int min, int max);
	// -------------------------------------------------------
	// Get random float between value - variance and value + variance
	// -------------------------------------------------------
	float randomRange(float value, float variance);
	// -------------------------------------------------------
	// Get random v2 between value - variance and value + variance
	// -------------------------------------------------------
	v2 randomRange(const v2& value, const v2& variance);
	// -------------------------------------------------------
	// get random between 0 - 100 and returns true if below min
	// -------------------------------------------------------
	bool chanceRoll(int min);
}