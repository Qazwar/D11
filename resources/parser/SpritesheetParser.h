#pragma once
#include "ResourceParser.h"
/*
spritesheet {
	name : "spritesheet"
}
*/
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