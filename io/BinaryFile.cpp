#include "BinaryFile.h"
#include <assert.h>
#include "..\utils\Assert.h"

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
			if (_file != 0) {
				ret = true;
			}
		}
		else {
			_file = fopen(fileName, "wb");
			if (_file != 0) {
				ret = true;
			}
		}
		_mode = mode;
		//if (mode == FileMode::READ) {
			// read entire file
			//ret = true;
		//}
		return ret;
	}


	void BinaryFile::write(int i) const {
		XASSERT(_file != 0,"Binary file is not open");
		XASSERT(_mode == FileMode::WRITE,"The file mode is not WRITE");
		fwrite(&i, sizeof(int), 1, _file);
	}

	void BinaryFile::write(char c) const {
		XASSERT(_file != 0, "Binary file is not open");
		XASSERT(_mode == FileMode::WRITE, "The file mode is not WRITE");
		fwrite(&c, sizeof(char), 1, _file);
	}

	void BinaryFile::write(uint16_t i) const {
		XASSERT(_file != 0, "Binary file is not open");
		XASSERT(_mode == FileMode::WRITE, "The file mode is not WRITE");
		fwrite(&i, sizeof(uint16_t), 1, _file);
	}

	void BinaryFile::write(uint32_t i) const {
		XASSERT(_file != 0, "Binary file is not open");
		XASSERT(_mode == FileMode::WRITE, "The file mode is not WRITE");
		fwrite(&i, sizeof(uint32_t), 1, _file);
	}

	void BinaryFile::write(float f) const {
		XASSERT(_file != 0, "Binary file is not open");
		XASSERT(_mode == FileMode::WRITE, "The file mode is not WRITE");
		fwrite(&f, sizeof(float), 1, _file);
	}

	void BinaryFile::write(const v3& v) const {
		XASSERT(_file != 0, "Binary file is not open");
		XASSERT(_mode == FileMode::WRITE, "The file mode is not WRITE");
		for (int i = 0; i < 3; ++i) {
			fwrite(&v.data[i], sizeof(float), 1, _file);
		}
	}

	void BinaryFile::write(const v2& v) const {
		XASSERT(_file != 0, "Binary file is not open");
		XASSERT(_mode == FileMode::WRITE, "The file mode is not WRITE");
		for (int i = 0; i < 2; ++i) {
			fwrite(&v.data[i], sizeof(float), 1, _file);
		}
	}

	void BinaryFile::write(const Color& v) const {
		XASSERT(_file != 0, "Binary file is not open");
		XASSERT(_mode == FileMode::WRITE, "The file mode is not WRITE");
		fwrite(&v.r, sizeof(float), 1, _file);
		fwrite(&v.g, sizeof(float), 1, _file);
		fwrite(&v.b, sizeof(float), 1, _file);
		fwrite(&v.a, sizeof(float), 1, _file);
	}

	void BinaryFile::write(void* data, int size) const {
		XASSERT(_file != 0, "Binary file is not open");
		XASSERT(_mode == FileMode::WRITE, "The file mode is not WRITE");
		fwrite(data, size, 1, _file);
	}

	void BinaryFile::read(int* v) const {
		XASSERT(_file != 0, "Binary file is not open");
		XASSERT(_mode == FileMode::READ, "The file mode is not READ");
		fread(v, sizeof(int), 1, _file);
	}

	void BinaryFile::read(char* v) const {
		XASSERT(_file != 0, "Binary file is not open");
		XASSERT(_mode == FileMode::READ, "The file mode is not READ");
		fread(v, sizeof(char), 1, _file);
	}

	void BinaryFile::read(uint32_t* v) const {
		XASSERT(_file != 0, "Binary file is not open");
		XASSERT(_mode == FileMode::READ, "The file mode is not READ");
		fread(v, sizeof(uint32_t), 1, _file);
	}

	void BinaryFile::read(void* data, int size) const {
		XASSERT(_file != 0, "Binary file is not open");
		XASSERT(_mode == FileMode::READ, "The file mode is not READ");
		fread(data, size, 1, _file);
	}

}