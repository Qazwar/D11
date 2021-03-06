#pragma once
#include "ResourceParser.h"
/*
font {
	name : "nullshock"
	file : "nullshock.json"
}
*/
namespace ds {

	namespace res {

		class FontParser : public ResourceParser {

		public:
			FontParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
			RID loadFont(const char* name, const BitmapfontDescriptor& descriptor);
		};

	}

}