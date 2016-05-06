#include "geometrics.h"
#include "..\math\math.h"

namespace ds {

	namespace geometrics {

		void createCube(Mesh* mesh, const Rect& textureRect,const v3& center, const v3& size) {
			float px = 0.5f * size.x;
			float py = 0.5f * size.y;
			float pz = 0.5f * size.z;
			ds::Texture t = math::buildTexture(textureRect);
			// front
			mesh->add(v3(center.x - px, center.y + py, center.z - pz), t.getUV(0));
			mesh->add(v3(center.x + px, center.y + py, center.z - pz), t.getUV(1));
			mesh->add(v3(center.x + px, center.y - py, center.z - pz), t.getUV(2));
			mesh->add(v3(center.x - px, center.y - py, center.z - pz), t.getUV(3));

			// right
			mesh->add(v3(center.x + px, center.y + py, center.z - pz), t.getUV(0));
			mesh->add(v3(center.x + px, center.y + py, center.z + pz), t.getUV(1));
			mesh->add(v3(center.x + px, center.y - py, center.z + pz), t.getUV(2));
			mesh->add(v3(center.x + px, center.y - py, center.z - pz), t.getUV(3));

			// top
			mesh->add(v3(center.x - px, center.y + py, center.z + pz), t.getUV(0));
			mesh->add(v3(center.x + px, center.y + py, center.z + pz), t.getUV(1));
			mesh->add(v3(center.x + px, center.y + py, center.z - pz), t.getUV(2));
			mesh->add(v3(center.x - px, center.y + py, center.z - pz), t.getUV(3));

			// left
			mesh->add(v3(center.x - px, center.y + py, center.z + pz), t.getUV(0));
			mesh->add(v3(center.x - px, center.y + py, center.z - pz), t.getUV(1));
			mesh->add(v3(center.x - px, center.y - py, center.z - pz), t.getUV(2));
			mesh->add(v3(center.x - px, center.y - py, center.z + pz), t.getUV(3));

			// back
			mesh->add(v3(center.x + px, center.y + py, center.z + pz), t.getUV(0));
			mesh->add(v3(center.x - px, center.y + py, center.z + pz), t.getUV(1));
			mesh->add(v3(center.x - px, center.y - py, center.z + pz), t.getUV(2));
			mesh->add(v3(center.x + px, center.y - py, center.z + pz), t.getUV(3));

			// bottom
			mesh->add(v3(center.x - px, center.y - py, center.z - pz), t.getUV(0));
			mesh->add(v3(center.x + px, center.y - py, center.z - pz), t.getUV(1));
			mesh->add(v3(center.x + px, center.y - py, center.z + pz), t.getUV(2));
			mesh->add(v3(center.x - px, center.y - py, center.z + pz), t.getUV(3));
		}
	}
}