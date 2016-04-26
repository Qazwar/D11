#pragma once
#include <stdint.h>

namespace ds {

	enum InternalEvents {
		ENGINE_SHUTDOWN
	};

	namespace events {

		void init();

		void reset();

		void send(uint32_t type);

		void send(uint32_t type, void* p, size_t size);

		bool get(uint32_t index, void* p);
		
		int getType(uint32_t index);

		bool containsType(uint32_t type);

		uint32_t num();

		void shutdown();

	}
}


