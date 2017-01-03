#pragma once
#include "ResourceParser.h"
/*
sound {
	file : "255.wav"
	name : "255"
}
*/
namespace ds {

	namespace res {

		class SceneParser : public ResourceParser {

		public:
			SceneParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
		};

	}

}