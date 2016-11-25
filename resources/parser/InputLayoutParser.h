#pragma once
#include "ResourceParser.h"

namespace ds {

	namespace res {

		class InputLayoutParser : public ResourceParser {

		public:
			InputLayoutParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
			RID createInputLayout(const char* name, const InputLayoutDescriptor& descriptor);
		};

	}

}