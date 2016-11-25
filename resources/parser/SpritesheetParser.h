#pragma once
#include "ResourceParser.h"

namespace ds {

	namespace res {

		class SpritesheetParser : public ResourceParser {

		public:
			SpritesheetParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
			RID loadSpriteSheet(const char* name);
		};

	}

}