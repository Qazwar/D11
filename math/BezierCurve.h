#pragma once
#include <Vector.h>

namespace ds {

	const int MAX_CURVE_SEGMENTS = 64;

	struct BezierCurve {

		Vector2f p0;
		Vector2f p1;
		Vector2f p2;
		Vector2f p3;
		float arcLengths[MAX_CURVE_SEGMENTS+1];
		float length;

		const void get(float t,Vector2f* p) const {
			float u = 1.0f - t;
			float tt = t * t;
			float uu = u * u;
			float uuu = uu * u;
			float ttt = tt * t;
			*p = uuu * p0;
			*p += 3 * uu * t * p1;
			*p += 3 * u * tt * p2;
			*p += ttt * p3;
		}

		const void getTangent(float t,Vector2f* p ) const {
			// dP(t) / dt =  -3(1-t)^2 * P0 + 3(1-t)^2 * P1 - 6t(1-t) * P1 - 3t^2 * P2 + 6t(1-t) * P2 + 3t^2 * P3 
			float u = 1.0f - t;
			float tt = t * t;
			float uu = u * u;
			float tu = t * u;
			*p = -3.0f * uu * p0;
			*p += 3.0f * uu * p1;
			*p -= 6.0f * tu * p1;
			*p -= 3.0f * tt * p2;
			*p += 6.0f * tu * p2;
			*p += 3.0f * tt * p3;		
			*p = normalize(*p);
		}

		void build() {
			arcLengths[0] = 0.0f;
			float ox = p0.x;
			float oy = p0.y;
			length = 0.0f;
			int num = 1;
			Vector2f now;
			float steps = 1.0f / MAX_CURVE_SEGMENTS;
			for (int i = 1; i <= MAX_CURVE_SEGMENTS; ++i) {
				float step = static_cast<float>(i) * steps ;
				get(step,&now);
				float dx = ox - now.x;
				float dy = oy - now.y;
				length += sqrt(dx * dx + dy * dy);
				arcLengths[num++] = length;
				ox = now.x;
				oy = now.y;
			}
		}

		const float getNormalized(float u) const {

			float targetLength = u * arcLengths[MAX_CURVE_SEGMENTS+1];
			int low = 0;
			int high = MAX_CURVE_SEGMENTS;
			int index = 0;
			while (low < high) {
				index = (low + (high - low) / 2);
				if ( arcLengths[index] < targetLength) {
					low = index + 1;

				} else {
					high = index;
				}
			}
			if ( arcLengths[index] > targetLength) {
				index--;
			}

			float lengthBefore = arcLengths[index];
			if (lengthBefore == targetLength) {
				return lengthBefore;

			} else {
				return (index + (targetLength - lengthBefore) / (arcLengths[index + 1] - lengthBefore)) / MAX_CURVE_SEGMENTS;
			}
		}

	};

	struct BezierCurve3D {

		Vector3f p0;
		Vector3f p1;
		Vector3f p2;
		Vector3f p3;
		float arcLengths[MAX_CURVE_SEGMENTS + 1];
		float length;

		const void get(float t, Vector3f* p) const {
			float u = 1.0f - t;
			float tt = t * t;
			float uu = u * u;
			float uuu = uu * u;
			float ttt = tt * t;
			*p = uuu * p0;
			*p += 3 * uu * t * p1;
			*p += 3 * u * tt * p2;
			*p += ttt * p3;
		}

		const void getTangent(float t, Vector3f* p) const {
			// dP(t) / dt =  -3(1-t)^2 * P0 + 3(1-t)^2 * P1 - 6t(1-t) * P1 - 3t^2 * P2 + 6t(1-t) * P2 + 3t^2 * P3 
			float u = 1.0f - t;
			float tt = t * t;
			float uu = u * u;
			float tu = t * u;
			*p = -3.0f * uu * p0;
			*p += 3.0f * uu * p1;
			*p -= 6.0f * tu * p1;
			*p -= 3.0f * tt * p2;
			*p += 6.0f * tu * p2;
			*p += 3.0f * tt * p3;
			*p = normalize(*p);
		}

		void build() {
			arcLengths[0] = 0.0f;
			float ox = p0.x;
			float oy = p0.y;
			float oz = p0.z;
			length = 0.0f;
			int num = 1;
			Vector3f now;
			float steps = 1.0f / MAX_CURVE_SEGMENTS;
			for (int i = 1; i <= MAX_CURVE_SEGMENTS; ++i) {
				float step = static_cast<float>(i)* steps;
				get(step, &now);
				float dx = ox - now.x;
				float dy = oy - now.y;
				float dz = oz - now.z;
				length += sqrt(dx * dx + dy * dy);
				arcLengths[num++] = length;
				ox = now.x;
				oy = now.y;
				oz = now.z;
			}
		}

		const float getNormalized(float u) const {

			float targetLength = u * arcLengths[MAX_CURVE_SEGMENTS + 1];
			int low = 0;
			int high = MAX_CURVE_SEGMENTS;
			int index = 0;
			while (low < high) {
				index = (low + (high - low) / 2);
				if (arcLengths[index] < targetLength) {
					low = index + 1;

				}
				else {
					high = index;
				}
			}
			if (arcLengths[index] > targetLength) {
				index--;
			}

			float lengthBefore = arcLengths[index];
			if (lengthBefore == targetLength) {
				return lengthBefore;

			}
			else {
				return (index + (targetLength - lengthBefore) / (arcLengths[index + 1] - lengthBefore)) / MAX_CURVE_SEGMENTS;
			}
		}

	};

}

