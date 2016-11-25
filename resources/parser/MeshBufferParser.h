#pragma once
#include "ResourceParser.h"

namespace ds {

	namespace res {

		class MeshBufferParser : public ResourceParser {

		public:
			MeshBufferParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
		private:
			RID createMeshBuffer(const char* name, const MeshBufferDescriptor& descriptor);
		};

	}

}