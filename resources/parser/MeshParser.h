#pragma once
#include "ResourceParser.h"

namespace ds {

	namespace res {

		class MeshParser : public ResourceParser {

		public:
			MeshParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
		private:
			RID createMesh(const char* name, const MeshDescriptor& descriptor);
		};

	}

}