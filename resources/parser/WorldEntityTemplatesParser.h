#pragma once
#include "ResourceParser.h"

namespace ds {

	namespace res {

		class WorldEntityTemplatesParser : public ResourceParser {

		public:
			WorldEntityTemplatesParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
		private:
			RID loadWorldEntityTemplates(const char* name);
		};

	}

}