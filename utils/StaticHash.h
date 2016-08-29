#pragma once
#include "..\Common.h"
#include "StringUtils.h"

#define SID(str) (ds::StaticHash(str))
//#define SID_VAL(str) (ds::string::murmur_hash(str))

namespace ds {

	class StringHash {

	private:
		uint32_t m_val;

		template<size_t N> inline uint32_t _Hash(const char(&str)[N]) const
		{
			typedef const char(&truncated_str)[N - 1];
			return str[N - 1] + 65599 * _Hash((truncated_str)str);
		}
		inline uint32_t _Hash(const char(&str)[2]) const { return str[1] + 65599 * str[0]; }

	public:
		template <size_t N> StringHash(const char(&str)[N]) { m_val = _Hash(str); }
		inline operator uint32_t() const { return m_val; }
	};

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
		const bool operator<(const ds::StaticHash &rhs) const {
			return _hash < rhs.get();
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

