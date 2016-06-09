#pragma once
#include <stdint.h>
#include "..\io\ReportWriter.h"

namespace ds {

	struct DrawCounter {

		uint32_t flushes;
		uint32_t vertices;
		uint32_t indices;

		void reset();

		void save(const ReportWriter& writer);
	};

	extern DrawCounter* gDrawCounter;

}
