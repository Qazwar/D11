#pragma once
#include "ResourceParser.h"
/*
world_entity_templates {
	name : "game_objects"
}
*/
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