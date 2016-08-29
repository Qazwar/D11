#pragma once
#include "..\lib\collection_types.h"
#include "..\Common.h"
#include "..\utils\StaticHash.h"

namespace ds {

	class GlobalStringBuffer {

		struct GSBEntry {
			StaticHash hash;
			int index;
			int size;
			bool used;
		};

	public:
		GlobalStringBuffer();
		~GlobalStringBuffer();
		int add(const char* str);
		const char* get(int index) const;
		void release(int index);
		bool contains(const char* str) const;
		int find(const char* str) const;
	private:
		CharBuffer _buffer;
		Array<GSBEntry> _entries;
	};

	extern GlobalStringBuffer* gStringBuffer;

}
