#pragma once
#include "ResourceParser.h"
/*
square_buffer {
	name : "Squares"
	texture : "TextureArray"
	size : 16384
}
*/
namespace ds {

	namespace res {

		class SquareBufferParser : public ResourceParser {

		public:
			SquareBufferParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
		private:
			RID createSquareBuffer(const char* name, int size, RID texture);
		};

	}

}