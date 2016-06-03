#pragma once
#include <stdio.h>
#include <Vector.h>
#include "..\utils\Color.h"
#include <stdint.h>

namespace ds {

	enum FileMode {
		READ,
		WRITE
	};

	class BinaryFile {

	public:
		BinaryFile();
		~BinaryFile();
		bool open(const char* name, FileMode mode);
		void write(int i);
		void write(uint16_t i);
		void write(uint32_t i);
		void write(float f);
		void write(const v3& v);
		void write(const v2& v);
		void write(const Color& color);
	private:
		FILE* _file;
		FileMode _mode;
	};

}

