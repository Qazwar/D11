#pragma once

namespace ds {

	namespace compression {

		char* encode(const char* text, int* size);

		char* decode(const char* text);

	}

}