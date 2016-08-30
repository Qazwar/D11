#pragma once
#include <stdint.h>

const uint32_t FNV_Prime = 0x01000193; //   16777619
const uint32_t FNV_Seed = 0x811C9DC5; // 2166136261

inline uint32_t fnv1a(const char* text, uint32_t hash = FNV_Seed) {
	const unsigned char* ptr = (const unsigned char*)text;
	while (*ptr) {
		hash = (*ptr++ ^ hash) * FNV_Prime;
	}
	return hash;
}

uint32_t murmur_hash(const char* text);
uint32_t murmur_hash(const void * key, int len, uint32_t seed);

#define SID(str) (StaticHash(str))
#define SID_VAL(str) (fnv1a(str))

// ----------------------------------------------------
// Static Hash 
// ----------------------------------------------------
class StaticHash {

public:
	StaticHash() {
		_hash = 0;
	}
	explicit StaticHash(const char* text) {
		_hash = fnv1a(text);
	}
	explicit StaticHash(uint32_t hash) {
		_hash = hash;
	}
	~StaticHash() {}
	const uint32_t get() const {
		return _hash;
	}
	const bool operator<(const StaticHash &rhs) const {
		return _hash < rhs.get();
	}
private:	
	uint32_t _hash;
};

const StaticHash INVALID_HASH = StaticHash(static_cast<unsigned int>(0));

const bool operator==(const StaticHash& lhs, const StaticHash &rhs);

const bool operator!=(const StaticHash& lhs, const StaticHash &rhs);



