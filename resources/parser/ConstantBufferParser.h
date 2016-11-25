#pragma once
#include "ResourceParser.h"

namespace ds {

	namespace res {

		class ConstantBufferParser : public ResourceParser {

		public:
			ConstantBufferParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
			RID createConsantBuffer(const char* name, const ConstantBufferDescriptor& descriptor);
		};

	}

}