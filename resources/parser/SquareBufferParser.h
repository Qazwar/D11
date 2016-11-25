#pragma once
#include "ResourceParser.h"

namespace ds {

	namespace res {

		class SquareBufferParser : public ResourceParser {

		public:
			SquareBufferParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
		private:
			RID createSquareBuffer(const char* name, int size, RID texture);
		};

	}

}