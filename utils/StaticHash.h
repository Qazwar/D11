#pragma once
#include "..\Common.h"
#include "StringUtils.h"

#define SID(str) (ds::StaticHash(str))
//#define SID_VAL(str) (ds::string::murmur_hash(str))

namespace ds {

	class StaticHash {

	public:
		StaticHash() {
			_hash = 0;
		}
		explicit StaticHash(const char* text) {
			_hash = murmur_hash(text);
		}
		explicit StaticHash(unsigned int hash) {
			_hash = hash;
		}
		~StaticHash() {}
		const unsigned int get() const {
			return _hash;
		}
	private:
		unsigned int murmur_hash(const char* text);
		unsigned int murmur_hash(const void * key, int len, unsigned int seed);
		unsigned int _hash;
	};

}

const bool operator==(const ds::StaticHash& lhs, const ds::StaticHash &rhs);

const bool operator<(const ds::StaticHash& lhs, const ds::StaticHash &rhs);

const bool operator!=(const ds::StaticHash& lhs, const ds::StaticHash &rhs);

