#pragma once
#include <Vector.h>

namespace ds {

	struct AABBox {

		v3 position;
		v3 extent;
		v3 min;
		v3 max;
		v3 bounds[2];

		AABBox() {}

		AABBox(const v3& pos, const v3& ext) {
			position = pos;
			extent = ext * 0.5f;
			min = vec_min(position - extent, position + extent);
			max = vec_max(position - extent, position + extent);
			bounds[0] = min;
			bounds[1] = max;
		}

		void scale(const v3& s) {
			extent.x *= s.x;
			extent.y *= s.y;
			min = vec_min(position - extent, position + extent);
			max = vec_max(position - extent, position + extent);
			bounds[0] = min;
			bounds[1] = max;
		}

		void transpose(const v3& pos) {
			position = pos;
			min = vec_min(position - extent, position + extent);
			max = vec_max(position - extent, position + extent);
			bounds[0] = min;
			bounds[1] = max;
		}

		v2 findClosestPoint(const v3& p) const {
			v2 ret(0, 0);
			ret.x = (p.x < min.x) ? min.x : (p.x > max.x) ? max.x : p.x;
			ret.y = (p.y < min.y) ? min.y : (p.y > max.y) ? max.y : p.y;
			return ret;
		}

		bool contains(const v3& point) const {
			if (point.x < min.x || max.x < point.x) {
				return false;
			}
			else if (point.y < min.y || max.y < point.y) {
				return false;
			}
			return true;
		}

		const bool overlaps(const AABBox& b) const {
			const v3 T = b.position - position;
			return fabs(T.x) <= (extent.x + b.extent.x) && fabs(T.y) <= (extent.y + b.extent.y) && fabs(T.z) <= (extent.z + b.extent.z);
		}
		/*
		const bool overlaps(const Sphere& sphere) const {
			float s, d = 0;
			//find the square of the distance
			//from the sphere to the box
			for (int i = 0; i < 2; ++i) {
				if (sphere.position[i] < min_value(i)) {
					s = sphere.position[i] - min_value(i);
					d += s*s;
				}
				else if (sphere.position[i] > max_value(i)) {
					s = sphere.position[i] - max_value(i);
					d += s*s;
				}

			}
			return d <= sphere.radius * sphere.radius;
		}
		*/
		float min_value(int index) const {
			return min[index];
		}

		float max_value(int index) const {
			return max[index];
		}

		// http://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
		bool intersects(const Ray& ray, float* t0, float* t1) const {

			float tmin = (bounds[ray.sign[0]].x - ray.origin.x) * ray.invDir.x;
			float tmax = (bounds[1 - ray.sign[0]].x - ray.origin.x) * ray.invDir.x;
			float tymin = (bounds[ray.sign[1]].y - ray.origin.y) * ray.invDir.y;
			float tymax = (bounds[1 - ray.sign[1]].y - ray.origin.y) * ray.invDir.y;

			if ((tmin > tymax) || (tymin > tmax))
				return false;
			if (tymin > tmin)
				tmin = tymin;
			if (tymax < tmax)
				tmax = tymax;

			float tzmin = (bounds[ray.sign[2]].z - ray.origin.z) * ray.invDir.z;
			float tzmax = (bounds[1 - ray.sign[2]].z - ray.origin.z) * ray.invDir.z;

			if ((tmin > tzmax) || (tzmin > tmax))
				return false;
			if (tzmin > tmin)
				tmin = tzmin;
			if (tzmax < tmax)
				tmax = tzmax;
			*t0 = tmin;
			*t1 = tmax;
			return true;
		}
	};

}