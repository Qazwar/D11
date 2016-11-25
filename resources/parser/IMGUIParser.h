#pragma once
#include "ResourceParser.h"

namespace ds {

	namespace res {

		class IMGUIParser : public ResourceParser {

		public:
			IMGUIParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);		
		};

	}

}