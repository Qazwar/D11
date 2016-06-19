#include "BinaryFile.h"
#include <assert.h>

namespace ds {

	BinaryFile::BinaryFile() : _file(0) {
	}


	BinaryFile::~BinaryFile() {
		if (_file != 0) {
			fclose(_file);
		}
	}

	bool BinaryFile::open(const char* fileName, FileMode mode) {
		bool ret = false;
		if (mode == FileMode::READ) {
			_file = fopen(fileName,"rb");
		}
		else {
			_file = fopen(fileName, "wb");
			if (_file != 0) {
				ret = true;
			}
		}
		_mode = mode;
		if (mode == FileMode::READ) {
			// read entire file
			ret = true;
		}
		return ret;
	}


	void BinaryFile::write(int i) {
		assert(_file != 0);
		assert(_mode == FileMode::WRITE);
		fwrite(&i, sizeof(int), 1, _file);
	}

	void BinaryFile::write(uint16_t i) {
		assert(_file != 0);
		assert(_mode == FileMode::WRITE);
		fwrite(&i, sizeof(uint16_t), 1, _file);
	}

	void BinaryFile::write(uint32_t i) {
		assert(_file != 0);
		assert(_mode == FileMode::WRITE);
		fwrite(&i, sizeof(uint32_t), 1, _file);
	}

	void BinaryFile::write(float f) {
		assert(_file != 0);
		assert(_mode == FileMode::WRITE);
		fwrite(&f, sizeof(float), 1, _file);
	}

	void BinaryFile::write(const v3& v) {
		assert(_file != 0);
		assert(_mode == FileMode::WRITE);
		for (int i = 0; i < 3; ++i) {
			fwrite(&v.data[i], sizeof(float), 1, _file);
		}
	}

	void BinaryFile::write(const v2& v) {
		assert(_file != 0);
		assert(_mode == FileMode::WRITE);
		for (int i = 0; i < 2; ++i) {
			fwrite(&v.data[i], sizeof(float), 1, _file);
		}
	}

	void BinaryFile::write(const Color& v) {
		assert(_file != 0);
		assert(_mode == FileMode::WRITE);
		fwrite(&v.r, sizeof(float), 1, _file);
		fwrite(&v.g, sizeof(float), 1, _file);
		fwrite(&v.b, sizeof(float), 1, _file);
		fwrite(&v.a, sizeof(float), 1, _file);
	}

	void BinaryFile::read(int* v) {
		assert(_file != 0);
		assert(_mode == FileMode::READ);
		fread(v, sizeof(int), 1, _file);
	}

	void BinaryFile::read(uint32_t* v) {
		assert(_file != 0);
		assert(_mode == FileMode::READ);
		fread(v, sizeof(uint32_t), 1, _file);
	}
	
}