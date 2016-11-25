#pragma once
#include "ResourceParser.h"

namespace ds {

	namespace res {

		class SpriteBufferParser : public ResourceParser {

		public:
			SpriteBufferParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
		private:
			RID createSpriteBuffer(const char* name, const SpriteBufferDescriptor& descriptor);
		};

	}

}