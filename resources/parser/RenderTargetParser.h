#pragma once
#include "ResourceParser.h"
/*
render_target {
	name : "RT1"
	width : 1024
	height : 768
	clear_color : 0,0,0,0
}
*/
namespace ds {

	namespace res {

		class RenderTargetParser : public ResourceParser {

		public:
			RenderTargetParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
		private:
			RID createRenderTarget(const char* name, const RenderTargetDescriptor& descriptor);
		};

	}

}