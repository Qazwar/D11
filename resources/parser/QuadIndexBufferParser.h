#pragma once
#include "ResourceParser.h"

namespace ds {

	namespace res {

		class QuadIndexBufferParser : public ResourceParser {

		public:
			QuadIndexBufferParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
		private:
			RID createQuadIndexBuffer(const char* name, const QuadIndexBufferDescriptor& descriptor);
		};

	}

}