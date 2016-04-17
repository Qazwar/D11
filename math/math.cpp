#include "math.h"
#include "..\utils\mtrand.h"

namespace math {

	// http://allenchou.net/2014/02/game-math-faster-sine-cosine-with-polynomial-curves/
	float hill(float x)	{
		const float a0 = 1.0f;
		const float a2 = 2.0f / PI - 12.0f / (PI * PI);
		const float a3 = 16.0f / (PI * PI * PI) - 4.0f / (PI * PI);
		const float xx = x * x;
		const float xxx = xx * x;
		return a0 + a2 * xx + a3 * xxx;
	}

	float fastSin(float x) {
		// wrap x within [0, TWO_PI)
		const float a = x * TWO_PI_INV;
		x -= static_cast<int>(a)* TWO_PI;
		if (x < 0.0f)
			x += TWO_PI;

		// 4 pieces of hills
		if (x < HALF_PI)
			return hill(HALF_PI - x);
		else if (x < PI)
			return hill(x - HALF_PI);
		else if (x < 3.0f * HALF_PI)
			return -hill(3.0f * HALF_PI - x);
		else
			return -hill(x - 3.0f * HALF_PI);
	}

	float fastCos(float x) {
		return fastSin(x + HALF_PI);
	}

	v4 getTextureCoordinates(const ds::Rect& textureRect, float textureWidth, float textureHeight) {
		v4 ret;
		ret.x = textureRect.left / textureWidth;
		ret.z = textureRect.right / textureWidth;
		ret.y = textureRect.top / textureHeight;
		ret.w = textureRect.bottom / textureHeight;
		return ret;
	}

	v4 getTextureCoordinates(float top, float left, float width, float height, float textureWidth, float textureHeight) {
		v4 ret;
		float tw = width / textureWidth;
		float th = height / textureHeight;
		ret.x = left / textureWidth;
		ret.z = ret.x + tw;
		ret.y = top / textureHeight;
		ret.w = ret.y + th;
		return ret;
	}

	ds::Texture buildTexture(float top, float left, float width, float height, float textureWidth, float textureHeight) {
		ds::Texture ret;
		ds::Rect r(top, left, width, height);
		ret.rect = r;
		ret.uv = getTextureCoordinates(r, textureWidth, textureHeight);
		ret.textureID = 0;
		ret.dim = Vector2f(width, height);
		ret.textureSize.x = textureWidth;
		ret.textureSize.y = textureHeight;
		return ret;
	}

	ds::Texture buildTexture(const ds::Rect& r, float textureWidth, float textureHeight) {
		ds::Texture ret;
		ret.rect = r;
		ret.uv = getTextureCoordinates(r, textureWidth, textureHeight);
		ret.textureID = 0;
		ret.dim = Vector2f(r.width(), r.height());
		ret.textureSize.x = textureWidth;
		ret.textureSize.y = textureHeight;
		return ret;
	}

	void rotate(Vector2f& v, float angle) {
		float xt = (v.x * cosf(angle)) + (v.y * sinf(angle));
		float yt = (v.y * cosf(angle)) - (v.x * sinf(angle));
		v.x = xt;
		v.y = yt;
	}

	float clamp(float value, float min, float max) {
		if (value < min) {
			return min;
		}
		if (value > max) {
			return max;
		}
		return value;
	}
	//! Scale and rotate counter clock wise and translate
	/*
	\param v the center
	\param u the position
	\param scaleX the x scale
	\param scaleY the y scale
	\param rotation the rotation in radians
	\return a new Vector2f which is scaled and rotated and translated
	*/
	Vector2f srt(const Vector2f& v, const Vector2f& u, float scaleX, float scaleY, float rotation) {
		float sx = u.x * scaleX;
		float sy = u.y * scaleY;

		// rotation clock wise
		//float xt = cosf(rotation) * sx + sinf(rotation) * sy;
		//float yt = -sinf(rotation) * sx + cosf(rotation) * sy;

		// rotation counter clock wise
		//float xt = cosf(rotation) * sx - sinf(rotation) * sy;
		//float yt = sinf(rotation) * sx + cosf(rotation) * sy;

		float xt = fastCos(rotation) * sx - fastSin(rotation) * sy;
		float yt = fastSin(rotation) * sx + fastCos(rotation) * sy;

		xt += v.x;
		yt += v.y;

		return Vector2f(xt, yt);
	}

	void srt(const Vector2f& v, const Vector2f& u, const Vector2f& scale, float rotation, Vector2f* ret) {
		float sx = u.x * scale.x;
		float sy = u.y * scale.y;

		// rotation clock wise
		//float xt = cosf(rotation) * sx + sinf(rotation) * sy;
		//float yt = -sinf(rotation) * sx + cosf(rotation) * sy;

		// rotation counter clock wise
		//float xt = cosf(rotation) * sx - sinf(rotation) * sy;
		//float yt = sinf(rotation) * sx + cosf(rotation) * sy;

		float xt = fastCos(rotation) * sx - fastSin(rotation) * sy;
		float yt = fastSin(rotation) * sx + fastCos(rotation) * sy;

		xt += v.x;
		yt += v.y;

		ret->x = xt;
		ret->y = yt;
		//return Vector2f(xt, yt);
	}

	Vector3f srt(const Vector3f& v, const Vector3f& u, float scaleX, float scaleY, float rotation) {
		float sx = u.x * scaleX;
		float sy = u.y * scaleY;

		// rotation clock wise
		//float xt = cosf(rotation) * sx + sinf(rotation) * sy;
		//float yt = -sinf(rotation) * sx + cosf(rotation) * sy;

		// rotation counter clock wise
		//float xt = cosf(rotation) * sx - sinf(rotation) * sy;
		//float yt = sinf(rotation) * sx + cosf(rotation) * sy;

		float xt = fastCos(rotation) * sx - fastSin(rotation) * sy;
		float yt = fastSin(rotation) * sx + fastCos(rotation) * sy;

		xt += v.x;
		yt += v.y;

		return Vector3f(xt, yt, v.z);
	}


	void addRadial(Vector2f& v, float radius, float angle) {
		v.x += radius * fastCos(angle);
		v.y += radius * fastSin(angle);
	}

	// -PI < beta < PI   0 < phi < 2PI
	void addRadial(Vector3f& v, float radius, float beta, float phi) {
		v.x += radius * cos(phi) * sin(beta);
		v.y += radius * sin(phi) * sin(beta);
		v.z += radius * cos(beta);
	}

	Vector2f calculateRadial(const Vector2f& v, float radius, float angle) {
		Vector2f ret;
		//float ra = math::reflect(angle);
		float ra = angle;
		ret.x = v.x + radius * fastCos(ra);
		ret.y = v.y + radius * fastSin(ra);
		return ret;
	}

	float calculateRotation(const Vector2f& v) {
		Vector2f vn = normalize(v);
		if (vn != V2_RIGHT) {
			float dt = clamp(dot(vn, V2_RIGHT), -1.0f, 1.0f);
			float tmp = acos(dt);
			float cross = -1.0f * vn.y;
			if (cross > 0.0f) {
				tmp = 2.0f * PI - tmp;
			}
			return tmp;
		}
		else {
			return 0.0f;
		}
	}

	float getAngle(const Vector2f& v1, const Vector2f& v2)  {
		Vector2f diff = v2 - v1;
		return calculateRotation(diff);
		/*
		Vector2f vn1 = normalize(v1);
		Vector2f vn2 = normalize(v2);
		if ( vn1 != vn2 ) {
		float dt = dot(vn1,vn2);
		if ( dt < -1.0f ) {
		dt = -1.0f;
		}
		if ( dt > 1.0f ) {
		dt = 1.0f;
		}
		float tmp = acos(dt);
		float cross = (vn1.x * vn2.y) - (vn2.x * vn1.y);
		if ( cross < 0.0f ) {
		tmp = 2.0f * PI - tmp;
		}
		return tmp;
		}
		else {
		return 0.0f;
		}
		*/
	}

	Vector2f getRadialVelocity(float angle, float velocity) {
		float vx = fastCos(angle) * velocity;
		float vy = fastSin(angle) * velocity;
		return Vector2f(vx, vy);
	}

	void clamp(Vector2f& v, const Vector2f& min, const Vector2f& max) {
		if (v.x > max.x) {
			v.x = max.x;
		}
		if (v.x < min.x) {
			v.x = min.x;
		}
		if (v.y > max.y) {
			v.y = max.y;
		}
		if (v.y < min.y) {
			v.y = min.y;
		}
	}

	float norm(float t, float max) {
		float r = t / max;
		if (r > 1.0f) {
			r = 1.0f;
		}
		if (r < 0.0f) {
			r = 0.0f;
		}
		return r;
	}

	// -------------------------------------------------------
	// random
	// -------------------------------------------------------
	float random(float min, float max) {
		MTRand_open rand;
		//rand.seed(GetTickCount());
		return min + (max - min)* (float)rand();
	}

	float randomRange(float value, float variance) {
		return random(value - variance, value + variance);
	}

	// -------------------------------------------------------
	// random v2
	// -------------------------------------------------------
	v2 random(const v2& min, const v2& max) {
		float x = random(min.x, max.x);
		float y = random(min.y, max.y);
		return v2(x, y);
	}

	// -------------------------------------------------------
	// random v2 range
	// -------------------------------------------------------
	v2 randomRange(const v2& value, const v2& variance) {
		float x = randomRange(value.x, variance.x);
		float y = randomRange(value.y, variance.y);
		return v2(x, y);
	}

	// -------------------------------------------------------
	// random int
	// -------------------------------------------------------
	int random(int min, int max) {
		MTRand_open rand;
		float minf = static_cast<float>(min);
		float maxf = static_cast<float>(max)+0.99f;
		float r = minf + (maxf - minf)* (float)rand();
		return static_cast<int>(r);
	}

	bool chanceRoll(int min) {
		int r = random(0, 100);
		return r <= min;
	}
}