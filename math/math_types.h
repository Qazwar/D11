#pragma once
#include <iostream>
#include "Vector.h"

namespace ds {

	const float PI = 3.141592654f;

	// -------------------------------------------------------
	// 3x3 Matrix
	// -------------------------------------------------------
	struct mat3 {

		union {
			struct {
				float        _11, _12, _13;
				float        _21, _22, _23;
				float        _31, _32, _33;
			};
			float m[3][3];
		};

		//Vector3f a,b,c;
		mat3();

		mat3(float m11, float m12, float m13, float m21, float m22, float m23, float m31, float m32, float m33);

		operator float *() const { return (float *)&_11; }
	};

	inline mat3::mat3() {
		for (int i = 0; i < 3; ++i) {
			for (int j = 0; j < 3; ++j) {
				m[i][j] = 0.0f;
			}
		}
	}


	inline mat3::mat3(float m11, float m12, float m13, float m21, float m22, float m23, float m31, float m32, float m33) {
		_11 = m11;
		_12 = m12;
		_13 = m13;
		_21 = m21;
		_22 = m22;
		_23 = m23;
		_31 = m31;
		_32 = m32;
		_33 = m33;
	}

	// -------------------------------------------------------
	// 4x4 Matrix
	// -------------------------------------------------------
	struct mat4 {

		union {
			struct {
				float        _11, _12, _13, _14;
				float        _21, _22, _23, _24;
				float        _31, _32, _33, _34;
				float        _41, _42, _43, _44;

			};
			float m[4][4];
		};

		mat4();
		mat4(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24, float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44);

		operator float *() const { return (float *)&_11; }
	};

	inline mat4::mat4() {
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				m[i][j] = 0.0f;
			}
		}
	}

	inline mat4::mat4(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24, float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44) {
		_11 = m11;
		_12 = m12;
		_13 = m13;
		_14 = m14;
		_21 = m21;
		_22 = m22;
		_23 = m23;
		_24 = m24;
		_31 = m31;
		_32 = m32;
		_33 = m33;
		_34 = m34;
		_41 = m41;
		_42 = m42;
		_43 = m43;
		_44 = m44;
	}

	struct Quaternion {
		v3 v;
		float w;

		Quaternion() : v(0, 0, 0), w(1.0f) {}

		Quaternion(const v3& vv, float ww) : v(vv), w(ww) {}

	};

	struct Ray {

		v3 origin;
		v3 direction;
		v3 invDir;
		int sign[3];

		Ray() : origin(0, 0, 0), direction(0, 0, 0), invDir(0, 0, 0) {
			for (int i = 0; i < 3; ++i) {
				sign[i] = 0;
			}
		}

		Ray(const v3& o, const v3& d) : origin(o), direction(d) {
			for (int i = 0; i < 3; ++i) {
				invDir.data[i] = 1.0f / direction.data[i];
				sign[i] = (invDir.data[i] < 0.0f);
			}
		}
	};

	// -------------------------------------------------------
	// Rect
	// -------------------------------------------------------
	struct Rect {

		float top, left, bottom, right;
		Rect();
		Rect(const Rect& rect);
		Rect(float top, float left, float width, float height);
		Rect(int _top, int _left, int _width, int _height);
		const float width() const;
		const float height() const;

		bool isInside(float x, float y) {
			if (x > right) {
				return false;
			}
			if (x < left) {
				return false;
			}
			if (y < bottom) {
				return false;
			}
			if (y > top) {
				return false;
			}
			return true;
		}
	};

	inline Rect::Rect() : top(0.0f), left(0.0f), bottom(0.0f), right(0.0f) {}

	inline Rect::Rect(float _top, float _left, float width, float height) : top(_top), left(_left) {
		bottom = _top + height;
		right = _left + width;
	}

	inline Rect::Rect(int _top, int _left, int _width, int _height) {
		top = static_cast<float>(_top);
		left = static_cast<float>(_left);
		bottom = top + static_cast<float>(_height);
		right = left + static_cast<float>(_width);
	}

	inline Rect::Rect(const Rect& rect) {
		top = rect.top;
		left = rect.left;
		bottom = rect.bottom;
		right = rect.right;
	}

	inline const float Rect::width() const {
		return right - left;
	}

	inline const float Rect::height() const {
		return bottom - top;
	}

	inline std::ostream& operator << (std::ostream& os, const Rect& rect) {
		os << rect.top;
		os << rect.left;
		os << rect.width();
		os << rect.height();
		return os;
	}

	inline std::istream& operator >> (std::istream& input, Rect& rect) {
		input >> rect.top;
		input >> rect.left;
		float w;
		input >> w;
		rect.right = rect.left + w;
		float h;
		input >> h;
		rect.bottom = rect.top + h;
		return input;
	}

	// -------------------------------------------------------
	// Circle
	// -------------------------------------------------------
	struct Circle {

		Vector2f pos;
		float radius;

		Circle() : pos(0.0f, 0.0f), radius(0.0f) {}

		Circle(const Vector2f& p, float r) : pos(p), radius(r) {}
	};

	struct Plane {

		v3 position;
		v3 normal;

		Plane() : position(0, 0, 0), normal(0, 0, 0) {}
		Plane(const v3& p, const v3& n) : position(p), normal(n) {}
	};
}