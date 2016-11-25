#pragma once
#include "ResourceParser.h"

namespace ds {

	namespace res {

		class ParticleManagerParser : public ResourceParser {

		public:
			ParticleManagerParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
		private:
			RID createParticleManager(const ParticleSystemsDescriptor& descriptor);
		};

	}

}