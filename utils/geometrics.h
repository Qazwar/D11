#pragma once
#include "..\renderer\MeshBuffer.h"

namespace ds {

	namespace geometrics {

		void createCube(Mesh* mesh,const Rect& textureRect,const v3& center = v3(0,0,0), const v3& size = v3(1,1,1), const v3& rotation = v3(0,0,0));

		void createCube(Mesh* mesh, Rect* textureRects, const v3& center = v3(0, 0, 0), const v3& size = v3(1, 1, 1), const v3& rotation = v3(0, 0, 0));

		void createGrid(Mesh* mesh, float cellSize, int countX, int countY, const Rect& textureRect,const v3& offset = v3(0,0,0), const Color& color = Color::WHITE);

		void createPlane(Mesh* mesh, const v3& position, const Rect& textureRect, const v2& size, float rotation = 0.0f, const Color& color = Color::WHITE);

	}
}

