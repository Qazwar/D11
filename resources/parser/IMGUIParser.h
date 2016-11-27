#pragma once
#include "ResourceParser.h"
/*
imgui {
	font : "gui_font"
	texture : "gui"
}
*/
namespace ds {

	namespace res {

		class IMGUIParser : public ResourceParser {

		public:
			IMGUIParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);		
		};

	}

}