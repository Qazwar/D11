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

		class SoundParser : public ResourceParser {

		public:
			SoundParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
		};

	}

}