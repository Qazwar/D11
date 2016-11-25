#pragma once
#include "ResourceParser.h"

namespace ds {

	namespace res {

		class BlendStateParser : public ResourceParser {

		public:
			BlendStateParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
			RID createBlendState(const char* name, const BlendStateDescriptor& descriptor);
		};

	}

}