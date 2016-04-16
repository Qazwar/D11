#pragma once

#include "..\renderer\render_types.h"

namespace math {

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

	float norm(float t, float max);
}