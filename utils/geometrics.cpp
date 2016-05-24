#include "geometrics.h"
#include "..\math\math.h"

namespace ds {

	namespace geometrics {

		void createCube(Mesh* mesh, const Rect& textureRect, const v3& center, const v3& size,const v3& rotation) {
			float px = 0.5f * size.x;
			float py = 0.5f * size.y;
			float pz = 0.5f * size.z;
			mat3 RX = matrix::mat3RotationX(rotation.x);
			mat3 RY = matrix::mat3RotationY(rotation.y);
			mat3 RZ = matrix::mat3RotationZ(rotation.z);
			ds::Texture t = math::buildTexture(textureRect);
			v3 points[] = {
				v3(-px, py, pz),v3(px, py, pz),v3(px, py, -pz),v3(-px, py, -pz),
				v3(-px, -py, pz),v3(px, -py, pz),v3(px, -py, -pz),v3(-px, -py, -pz)
			};
			// front right top left back bottom
			v3 norms[] = { v3(0, 0, -1), v3(1,0,0), v3(0,1,0), v3(-1,0,0), v3(0,0,1) , v3(0,-1,0)};
			int indices[] = { 3, 2, 6, 7, 2, 1, 5, 6, 0, 1, 2, 3, 0, 3, 7, 4, 1, 0, 4, 5, 7, 6, 5, 4 };
			for (int i = 0; i < 6; ++i) {
				for (int j = 0; j < 4; ++j) {
					v3 p = points[indices[i * 4 + j]];
					p = RZ * RY * RX * p + center;
					v3 nn = RZ * RY * RX * norms[i];
					mesh->add(p, nn, t.getUV(j));
				}
			}
		}

		void createCube(Mesh* mesh, Rect* textureRects, const v3& center, const v3& size, const v3& rotation) {
			float px = 0.5f * size.x;
			float py = 0.5f * size.y;
			float pz = 0.5f * size.z;
			mat3 RX = matrix::mat3RotationX(rotation.x);
			mat3 RY = matrix::mat3RotationY(rotation.y);
			mat3 RZ = matrix::mat3RotationZ(rotation.z);
			ds::Texture t[6];
			for (int i = 0; i < 6; ++i) {
				t[i] = math::buildTexture(textureRects[i]);
			}
			v3 points[] = {
				v3(-px, py, pz), v3(px, py, pz), v3(px, py, -pz), v3(-px, py, -pz),
				v3(-px, -py, pz), v3(px, -py, pz), v3(px, -py, -pz), v3(-px, -py, -pz)
			};
			// front right top left back bottom
			v3 norms[] = { v3(0, 0, -1), v3(1, 0, 0), v3(0, 1, 0), v3(-1, 0, 0), v3(0, 0, 1), v3(0, -1, 0) };
			int indices[] = { 3, 2, 6, 7, 2, 1, 5, 6, 0, 1, 2, 3, 0, 3, 7, 4, 1, 0, 4, 5, 7, 6, 5, 4 };
			for (int i = 0; i < 6; ++i) {
				for (int j = 0; j < 4; ++j) {
					v3 p = points[indices[i * 4 + j]];
					p = RZ * RY * RX * p + center;
					v3 nn = RZ * RY * RX * norms[i];
					mesh->add(p, nn, t[i].getUV(j));
				}
			}
		}

		// ---------------------------------------------------------------
		// create XZ grid
		// ---------------------------------------------------------------
		void createGrid(Mesh* mesh, float cellSize, int countX, int countY, const Rect& textureRect, const v3& offset,const Color& color)  {
			v3 center = offset;
			center.z = offset.z + cellSize * 0.5f;
			float px = 0.5f * cellSize;
			float py = 0.0f;
			float pz = 0.5f * cellSize;
			ds::Texture t = math::buildTexture(textureRect);
			v3 norm = v3(0, 1, 0);
			v3 points[] = {
				v3(-px, offset.y,  pz),
				v3( px, offset.y,  pz),
				v3( px, offset.y, -pz),
				v3(-px, offset.y, -pz)
			};
			for (int y = 0; y < countY; ++y) {
				center.x = offset.x + cellSize * 0.5f;
				for (int x = 0; x < countX; ++x) {
					for (int i = 0; i < 4; ++i) {
						v3 n = points[i] + center;
						mesh->add(n, norm, t.getUV(i), color);
					}
					center.x += cellSize;
				}
				center.z += cellSize;
			}
		}

		// ---------------------------------------------------------------
		// create XZ plane
		// ---------------------------------------------------------------
		void createPlane(Mesh* mesh, const v3& position, const Rect& textureRect, const v2& size, float rotation, const Color& color)  {
			v3 center = position;
			mat3 R = matrix::mat3RotationY(rotation);
			float px = 0.5f * size.x;
			float py = 0.0f;
			float pz = 0.5f * size.y;
			ds::Texture t = math::buildTexture(textureRect);
			v3 points[] = {
				v3( -px, 0.0f, pz),
				v3(px, 0.0f, pz),
				v3(px, 0.0f, -pz),
				v3(-px, 0.0f, -pz)
			};
			for (int i = 0; i < 4; ++i) {
				v3 n = R * points[i];
				n += center;
				mesh->add(n, v3(0, 1, 0), t.getUV(i), color);
			}
		}

		// ---------------------------------------------------------------
		// create XZ plane
		// ---------------------------------------------------------------
		void createXYPlane(Mesh* mesh, const v3& position, const Rect& textureRect, const v2& size, float rotation, const Color& color) {
			v3 center = position;
			mat3 R = matrix::mat3RotationY(rotation);
			float px = 0.5f * size.x;
			float py = 0.5f * size.y;
			float pz = position.z;
			ds::Texture t = math::buildTexture(textureRect);
			v3 points[] = {
				v3(-px,  py, pz),
				v3( px,  py, pz),
				v3( px, -py, pz),
				v3(-px, -py, pz)
			};
			for (int i = 0; i < 4; ++i) {
				v3 n = R * points[i];
				n += center;
				mesh->add(n, v3(0, 0, -1), t.getUV(i), color);
			}
		}

	}
}