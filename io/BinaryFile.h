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
		void write(int i) const;
		void write(char c) const;
		void write(uint16_t i) const;
		void write(uint32_t i) const;
		void write(float f) const;
		void write(const v3& v) const;
		void write(const v2& v) const;
		void write(const Color& color) const;
		void write(void* data, int size) const;
		void read(int* v) const;
		void read(char* v) const;
		void read(uint32_t* v) const;
		void read(void* data, int size) const;
	private:
		FILE* _file;
		FileMode _mode;
	};

}

