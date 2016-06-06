#pragma once
#include "..\lib\collection_types.h"
#include <stdint.h>

namespace ds {

	class TileMapReader {

	public:
		TileMapReader();
		~TileMapReader();
		bool parse(const char* fileName);
		uint16_t width() const;
		uint16_t height() const;
		uint16_t get(uint16_t x, uint16_t y) const;
		uint16_t get(uint16_t index) const;
	private:
		ds::Array<uint16_t> _data;
		uint16_t _sizeX;
		uint16_t _sizeY;
	};

}