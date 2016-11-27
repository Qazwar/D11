#pragma once
#include "ResourceParser.h"
/*
sampler_state {
	name : "ClampSamplerState"
	addressU : "CLAMP"
	addressV : "CLAMP"
	addressW : "CLAMP"
}
*/
namespace ds {

	namespace res {

		class SamplerStateParser : public ResourceParser {

		public:
			SamplerStateParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
			RID createSamplerState(const char* name, const SamplerStateDescriptor& descriptor);
		};

	}

}