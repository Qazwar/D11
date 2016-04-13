#include "GlobalStringBuffer.h"
#include "StringUtils.h"

namespace ds {

	GlobalStringBuffer* gStringBuffer;

	GlobalStringBuffer::GlobalStringBuffer() {
	}


	GlobalStringBuffer::~GlobalStringBuffer() {
	}

	bool GlobalStringBuffer::contains(const char* str) const {
		return find(str) != -1;
	}

	int GlobalStringBuffer::find(const char* str) const {
		IdString hash = string::murmur_hash(str);
		for (int i = 0; i < _entries.size(); ++i) {
			if (_entries[i].hash == hash && !_entries[i].used) {
				return i;
			}
		}
		return -1;
	}

	int GlobalStringBuffer::add(const char* str) {
		int index = find(str);
		if (index == -1) {
			index = _buffer.size;
			int l = strlen(str);
			if (_buffer.size + l > _buffer.capacity) {
				_buffer.resize(_buffer.capacity + l + 256);
			}
			_buffer.append(str, l);
			GSBEntry entry;
			entry.hash = string::murmur_hash(str);
			entry.index = index;
			entry.size = l;
			entry.used = true;
			_entries.push_back(entry);
		}
		return index;
	}

	const char* GlobalStringBuffer::get(int index) const {
		return _buffer.data + index;
	}

	void GlobalStringBuffer::release(int index) {
		// ????
	}
}
