#pragma once
#include "ResourceParser.h"

namespace ds {

	namespace res {

		class SoundParser : public ResourceParser {

		public:
			SoundParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
		};

	}

}