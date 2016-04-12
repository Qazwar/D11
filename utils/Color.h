#pragma once
#include <stdint.h>

struct Color {

	union {
		float values[4];
		struct {
			float r;
			float g;
			float b;
			float a;
		};
	};

	Color() : r(1.0f) , g(1.0f) , b(1.0f) , a(1.0f) {}
	Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
	Color(const Color& v);
	Color(float* colorValues) {
		for (int i = 0; i < 4; ++i) {
			values[i] = colorValues[i];
		}
	}

	operator float* () {
		return &values[0];
	}

	operator const float* () const {
		return &values[0];
	}

	void operator = (const Color& v) {
		r = v.r;
		g = v.g;
		b = v.b;
		a = v.a;
	}	
};

inline Color::Color(const Color& v) {
	r = v.r;
	g = v.g;
	b = v.b;
	a = v.a;
}

inline Color::Color(uint8_t rc, uint8_t gc, uint8_t bc, uint8_t ac) {
	r = (float)rc / 255.0f;
	g = (float)gc / 255.0f;
	b = (float)bc / 255.0f;
	a = (float)ac / 255.0f;
}

const Color WHITE = Color(255, 255, 255, 255);
const Color RED   = Color(255,   0,   0, 255);