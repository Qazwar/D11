#pragma once
#include "ResourceParser.h"

namespace ds {

	namespace res {

		class VertexBufferParser : public ResourceParser {

		public:
			VertexBufferParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
			RID createVertexBuffer(const char* name, const VertexBufferDescriptor& descriptor);
		};

	}

}