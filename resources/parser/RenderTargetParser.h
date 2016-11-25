#pragma once
#include "ResourceParser.h"

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