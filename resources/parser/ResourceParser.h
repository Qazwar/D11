#pragma once
#include "..\ResourceContainer.h"

namespace ds {

	namespace res {

		class ResourceParser {

		public:
			ResourceParser(ResourceContext* ctx) : _resCtx(ctx) {}
			virtual RID parse(JSONReader& reader, int childIndex) = 0;
		protected:
			RID create(const char* name, ResourceType type);
			ResourceContext* _resCtx;
		};

	}

}