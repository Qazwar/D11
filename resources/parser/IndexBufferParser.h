#pragma once
#include "ResourceParser.h"

namespace ds {

	namespace res {

		class IndexBufferParser : public ResourceParser {

		public:
			IndexBufferParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
		private:
			RID createIndexBuffer(const char* name, const IndexBufferDescriptor& descriptor);
		};

	}

}