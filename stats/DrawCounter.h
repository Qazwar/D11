#pragma once
#include <stdint.h>
#include "core\io\ReportWriter.h"

namespace ds {

	struct DrawCounter {

		uint32_t flushes;
		uint32_t vertices;
		uint32_t indices;
		uint32_t sprites;
		uint32_t spriteFlushes;
		uint32_t squares;

		void reset();

		void save(const ReportWriter& writer);
	};

	extern DrawCounter* gDrawCounter;

}
