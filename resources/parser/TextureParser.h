#pragma once
#include "ResourceParser.h"

namespace ds {

	namespace res {

		class TextureParser : public ResourceParser {

		public:
			TextureParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
		private:
			RID loadTexture(const char* name, const TextureDescriptor& descriptor);
		};

	}

}