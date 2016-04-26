#include "EventStream.h"
#include <stdint.h>
#include "..\lib\collection_types.h"
#include "..\utils\Assert.h"

namespace ds {

	namespace events {

		const int EVENT_HEADER_SIZE = 12;

		struct EventHeader {
			uint32_t id;
			uint32_t type;
			size_t size;
		};

		struct EventContext {
			char* data;
			Array<uint32_t> mappings;
			uint32_t index;
		};

		static EventContext* _eventCtx = 0;

		void init() {
			_eventCtx = new EventContext;
			_eventCtx->data = (char*)ALLOC(4096);
			reset();
		}

		void reset() {
			_eventCtx->mappings.clear();
			_eventCtx->index = 0;
		}

		void shutdown() {
			if (_eventCtx != 0) {
				DEALLOC(_eventCtx->data);
				delete _eventCtx;
			}
		}

		// -------------------------------------------------------
		// add header
		// -------------------------------------------------------
		static void addHeader(uint32_t type, size_t size) {
			EventHeader header;
			header.id = _eventCtx->mappings.size();;
			header.size = size;
			header.type = type;
			char* data = _eventCtx->data + _eventCtx->index;
			memcpy(data, &header, EVENT_HEADER_SIZE);
		}

		// -------------------------------------------------------
		// add event
		// -------------------------------------------------------
		void send(uint32_t type) {
			addHeader(type, 0);
			char* data = _eventCtx->data + _eventCtx->index;
			_eventCtx->mappings.push_back(_eventCtx->index);
			_eventCtx->index += EVENT_HEADER_SIZE;
		}

		// -------------------------------------------------------
		// add event
		// -------------------------------------------------------
		void send(uint32_t type, void* p, size_t size) {
			addHeader(type, size);
			char* data = _eventCtx->data + _eventCtx->index + EVENT_HEADER_SIZE;
			memcpy(data, p, size);
			_eventCtx->mappings.push_back(_eventCtx->index);
			_eventCtx->index += EVENT_HEADER_SIZE + size;
		}

		// -------------------------------------------------------
		// get
		// -------------------------------------------------------
		bool get(uint32_t index, void* p) {
			XASSERT(index < _eventCtx->mappings.size(), "Index out of range");
			int lookup = _eventCtx->mappings[index];
			char* data = _eventCtx->data + lookup;
			EventHeader* header = (EventHeader*)data;
			data += EVENT_HEADER_SIZE;
			memcpy(p, data, header->size);
			return true;
		}

		// -------------------------------------------------------
		// get type
		// -------------------------------------------------------
		int getType(uint32_t index) {
			XASSERT(index < _eventCtx->mappings.size(), "Index out of range");
			int lookup = _eventCtx->mappings[index];
			char* data = _eventCtx->data + lookup;
			EventHeader* header = (EventHeader*)data;
			return header->type;
		}

		// -------------------------------------------------------
		// contains type
		// -------------------------------------------------------
		bool containsType(uint32_t type) {
			for (int i = 0; i < _eventCtx->mappings.size(); ++i) {
				if (getType(i) == type) {
					return true;
				}
			}
			return false;
		}

		uint32_t num() {
			return _eventCtx->mappings.size();
		}
	}
}