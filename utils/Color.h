#pragma once
#include <stdint.h>

namespace ds {

	// ------------------------------------------------------
	// HSL
	// 0 < h < 360
	// 0 < s < 100
	// 0 < l < 100
	// ------------------------------------------------------
	struct HSL {

		union {
			float values[3];
			struct {
				float h;
				float s;
				float l;
			};
		};

		HSL() : h(0.0f), s(0.0f), l(0.0f) {}
		HSL(float hue, float saturation, float luminance) : h(hue), s(saturation), l(luminance) {}

	};

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

		Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
		Color(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
		Color(float _r, float _g, float _b) : r(_r), g(_g), b(_b), a(1.0f) {}
		Color(int r, int g, int b, int a);
		Color(int r, int g, int b);
		Color(int* array);
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

		static const Color WHITE;
	};

	

	inline Color::Color(const Color& v) {
		r = v.r;
		g = v.g;
		b = v.b;
		a = v.a;
	}

	inline Color::Color(int rc, int gc, int bc, int ac) {
		r = (float)rc / 255.0f;
		g = (float)gc / 255.0f;
		b = (float)bc / 255.0f;
		a = (float)ac / 255.0f;
	}

	inline Color::Color(int rc, int gc, int bc) {
		r = (float)rc / 255.0f;
		g = (float)gc / 255.0f;
		b = (float)bc / 255.0f;
		a = 1.0f;
	}

	inline Color::Color(int* array) {
		for (int i = 0; i < 4; ++i) {
			values[i] = (float)array[i] / 255.0f;
		}
	}

	namespace color {

		Color convert(const HSL& hsl);

		Color lerp(const Color& lhs, const Color& rhs, float t);

		Color hsvToColor(float h, float s, float v);

		void generateGoldenRatioRainbow(int count, float s, float l, Color* colors, int num);

	}
}

ds::Color operator*=(ds::Color lhs, const ds::Color& rhs);

ds::Color operator*(ds::Color lhs, const ds::Color& rhs);