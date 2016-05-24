#include "Color.h"
#include <algorithm>

const ds::Color ds::Color::WHITE(1.0f, 1.0f, 1.0f, 1.0f);

ds::Color operator*=(ds::Color lhs, const ds::Color& rhs) {
	for (int i = 0; i < 4; ++i) {
		lhs.values[i] *= rhs.values[i];
	}
	return lhs;
}

ds::Color operator*(ds::Color lhs, const ds::Color& rhs) {
	return lhs *= rhs;
}

namespace ds {

	namespace color {

		Color lerp(const Color& lhs, const Color& rhs, float t) {
			if (t <= 0.0f) {
				return lhs;
			}
			if (t >= 1.0f) {
				return rhs;
			}
			float invT = 1.0f - t;
			return Color(lhs.r * invT + rhs.r * t, lhs.g * invT + rhs.g * t, lhs.b * invT + rhs.b * t, lhs.a * invT + rhs.a * t);
		}

		// 0 < h < 360 / 0 < s < 100 / 0 < v < 100
		Color hsvToColor(float h, float s, float v) {
			if (h == 0 && s == 0) {
				return Color(v / 100.0f, v / 100.0f, v / 100.0f);
			}

			float hh = h / 60.0f;

			float c = s / 100.0f * v / 100.0f;
			float x = c * (1.0f - std::abs(fmod(hh, 2.0f) - 1.0f));
			Color clr;
			if (hh < 1.0f) {
				clr = Color(c, x, 0.0f);
			}
			else if (hh < 2.0f) {
				clr = Color(x, c, 0.0f);
			}
			else if (hh < 3.0f) {
				clr = Color(0.0f, c, x);
			}
			else if (hh < 4.0f) {
				clr = Color(0.0f, x, c);
			}
			else if (hh < 5) {
				clr = Color(x, 0.0f, c);
			}
			else {
				clr = Color(c, 0.0f, x);
			}
			float m = v / 100.0f - c;
			clr.r += m;
			clr.g += m;
			clr.b += m;
			return clr;
		}
	}

}