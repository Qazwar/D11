#pragma once
#include "ResourceParser.h"

namespace ds {

	namespace res {

		class MaterialParser : public ResourceParser {

		public:
			MaterialParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
			RID createMaterial(const char* name, const MaterialDescriptor& descriptor);		
		};

	}

}