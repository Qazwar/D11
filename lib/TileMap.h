#pragma once

namespace ds {

	template<class T>
	class TileMap {

		struct TileMapNode {
			T data;

		};

	public:
		TileMap(int width, int height) {

		}

		~TileMap() {

		}

	private:
		int _width;
		int _height;
	};

}