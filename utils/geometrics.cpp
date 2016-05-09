#include "geometrics.h"
#include "..\math\math.h"

namespace ds {

	namespace geometrics {

		void createCube(Mesh* mesh, const Rect& textureRect, const v3& center, const v3& size) {
			float px = 0.5f * size.x;
			float py = 0.5f * size.y;
			float pz = 0.5f * size.z;
			ds::Texture t = math::buildTexture(textureRect);
			// front
			mesh->add(v3(center.x - px, center.y + py, center.z - pz), v3(0, 0, -1), t.getUV(0));
			mesh->add(v3(center.x + px, center.y + py, center.z - pz), v3(0, 0, -1), t.getUV(1));
			mesh->add(v3(center.x + px, center.y - py, center.z - pz), v3(0, 0, -1), t.getUV(2));
			mesh->add(v3(center.x - px, center.y - py, center.z - pz), v3(0, 0, -1), t.getUV(3));

			// right
			mesh->add(v3(center.x + px, center.y + py, center.z - pz), v3(1, 0, 0), t.getUV(0));
			mesh->add(v3(center.x + px, center.y + py, center.z + pz), v3(1, 0, 0), t.getUV(1));
			mesh->add(v3(center.x + px, center.y - py, center.z + pz), v3(1, 0, 0), t.getUV(2));
			mesh->add(v3(center.x + px, center.y - py, center.z - pz), v3(1, 0, 0), t.getUV(3));

			// top
			mesh->add(v3(center.x - px, center.y + py, center.z + pz), v3(0, 1, 0), t.getUV(0));
			mesh->add(v3(center.x + px, center.y + py, center.z + pz), v3(0, 1, 0), t.getUV(1));
			mesh->add(v3(center.x + px, center.y + py, center.z - pz), v3(0, 1, 0), t.getUV(2));
			mesh->add(v3(center.x - px, center.y + py, center.z - pz), v3(0, 1, 0), t.getUV(3));

			// left
			mesh->add(v3(center.x - px, center.y + py, center.z + pz), v3(-1, 0, 0), t.getUV(0));
			mesh->add(v3(center.x - px, center.y + py, center.z - pz), v3(-1, 0, 0), t.getUV(1));
			mesh->add(v3(center.x - px, center.y - py, center.z - pz), v3(-1, 0, 0), t.getUV(2));
			mesh->add(v3(center.x - px, center.y - py, center.z + pz), v3(-1, 0, 0), t.getUV(3));

			// back
			mesh->add(v3(center.x + px, center.y + py, center.z + pz), v3(0, 0, 1), t.getUV(0));
			mesh->add(v3(center.x - px, center.y + py, center.z + pz), v3(0, 0, 1), t.getUV(1));
			mesh->add(v3(center.x - px, center.y - py, center.z + pz), v3(0, 0, 1), t.getUV(2));
			mesh->add(v3(center.x + px, center.y - py, center.z + pz), v3(0, 0, 1), t.getUV(3));

			// bottom
			mesh->add(v3(center.x - px, center.y - py, center.z - pz), v3(0, -1, 0), t.getUV(0));
			mesh->add(v3(center.x + px, center.y - py, center.z - pz), v3(0, -1, 0), t.getUV(1));
			mesh->add(v3(center.x + px, center.y - py, center.z + pz), v3(0, -1, 0), t.getUV(2));
			mesh->add(v3(center.x - px, center.y - py, center.z + pz), v3(0, -1, 0), t.getUV(3));
		}


		void createGrid(Mesh* mesh, float cellSize, int countX, int countY, const Rect& textureRect, const v3& offset)  {
			v3 center = offset;
			float px = 0.5f * cellSize;
			float py = 0.0f;// 0.5f * cellSize;
			float pz = 0.5f * cellSize;
			ds::Texture t = math::buildTexture(textureRect);
			for (int y = 0; y < countY; ++y) {
				center.x = offset.x;
				for (int x = 0; x < countX; ++x) {
					mesh->add(v3(center.x - px, center.y + py, center.z + pz), v3(0, 1, 0), t.getUV(0), Color(255, 0, 0, 255));
					mesh->add(v3(center.x + px, center.y + py, center.z + pz), v3(0, 1, 0), t.getUV(1), Color(255, 0, 0, 255));
					mesh->add(v3(center.x + px, center.y + py, center.z - pz), v3(0, 1, 0), t.getUV(2), Color(255, 0, 0, 255));
					mesh->add(v3(center.x - px, center.y + py, center.z - pz), v3(0, 1, 0), t.getUV(3), Color(255, 0, 0, 255));
					center.x += cellSize;
				}
				center.z += cellSize;
			}
		}

	}
}