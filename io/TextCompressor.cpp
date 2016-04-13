#include "TextCompressor.h"
#include "..\utils\Log.h"
#include <math.h>
#include <stdint.h>
#include <string.h>

namespace ds {

	namespace compression {

		unsigned int nextPowerOf2(unsigned int n) {
			n--;
			n |= n >> 1;
			n |= n >> 2;
			n |= n >> 4;
			n |= n >> 8;
			n |= n >> 16;
			n++;
			return n;
		}

		bool contains(const char c, const char* array, uint8_t length) {
			for (uint8_t i = 0; i < length; ++i) {
				if (array[i] == c) {
					return true;
				}
			}
			return false;
		}

		uint8_t extract_used_characters(const char* text, char* array, uint8_t max) {
			const char* p = text;
			uint8_t cnt = 0;
			while (*p) {
				if (!contains(*p, array, cnt)) {
					array[cnt++] = *p;
				}
				++p;
			}
			return cnt;
		}

		void to_binary(uint8_t c, char* buffer, int offset, uint8_t bits) {
			int tmp = pow(2.0f, bits - 1);
			int current = offset;
			int cnt = 0;
			for (int i = tmp; i > 0; i >>= 1) {
				if ((c & i) == i) {
					buffer[current++] = '1';
				}
				else {
					buffer[current++] = '0';
				}
			}
			buffer[current++] = '\0';
		}

		uint8_t find_index(const char c, char* table, int max) {
			for (uint8_t i = 0; i < max; ++i) {
				if (table[i] == c) {
					return i;
				}
			}
			return 0;
		}

		uint8_t from_binary(const char* text, int offset, int bits) {
			uint8_t ret = 0;
			int tmp = pow(2.0f, bits - 1);
			int current = offset;
			for (int i = 0; i < bits; ++i) {
				if (text[current++] == '1') {
					ret |= 1 << ((bits - 1) - i);
				}
			}
			return ret;
		}

		char from_binary(uint8_t c, int bits) {
			unsigned char ret = 0;
			int tmp = pow(2.0f, bits - 1);
			for (int i = bits; i > 0; --i) {
				int v = 1 << i;
				if ((c & v) == v) {
					ret |= 1 << i;
				}
			}
			return ret;
		}

		char* encode(const char* text,int* size) {
			char table[256];
			uint8_t num = extract_used_characters(text, table, 256);
			uint8_t bits = nextPowerOf2(num);			
			int tmp = bits;
			uint8_t numBits = 0;
			while (tmp > 0) {
				++numBits;
				tmp /= 2;
			}
			if (numBits > 0) {
				--numBits;
			}
			LOG << "characters used: " << static_cast<int>(num) << " next po2: " << static_cast<int>(bits) << " num bits: " << static_cast<int>(numBits);
			int len = strlen(text);
			char* buffer = new char[len * numBits + 2];
			int offset = 0;
			const char* ptr = text;
			while (*ptr) {
				uint8_t idx = find_index(*ptr, table, num);
				to_binary(idx, buffer, offset, numBits);
				offset += numBits;
				++ptr;
			}
			int totalSize = len + 4 + num;
			char* result = new char[totalSize + 1];
			int current = 0;
			result[current++] = numBits;
			result[current++] = num;
			for (int i = 0; i < num; ++i) {
				result[current++] = table[i];
			}
			result[current++] = len & 0xff;
			result[current++] = (len >> 8) & 0xff;
			offset = 0;
			for (int i = 0; i < len; ++i) {
				result[current++] = from_binary(buffer, offset, numBits);
				offset += numBits;
			}
			LOG << "text length: " << len << " total size: " << totalSize;
			*size = totalSize + 1;
			result[totalSize] = '\0';
			delete[] buffer;
			return result;
		}

		char* decode(const char* text) {
			char table[256];
			int current = 0;
			uint8_t numBits = text[current++];
			uint8_t num = text[current++];
			for (uint8_t i = 0; i < num; ++i) {
				table[i] = text[current++];
			}
			uint8_t low = text[current++];
			uint8_t high = text[current++];			
			int total = (high << 8) + low;
			char* buffer = new char[total * numBits + 2];
			int offset = 0;
			unsigned char p = 0;
			for (int i = 0; i < total; ++i) {
				p = text[current++];
				to_binary(p, buffer, offset, numBits);
				offset += numBits;
			}
			char* result = new char[total + 1];
			offset = 0;
			for (int i = 0; i < total; ++i) {
				unsigned char p = from_binary(buffer, offset, numBits);
				offset += numBits;
				result[i] = table[p];
			}
			result[total] = '\0';
			delete[] buffer;
			return result;
		}

	}

}