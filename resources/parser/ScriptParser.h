#pragma once
#include "ResourceParser.h"

namespace ds {

	namespace res {

		class ScriptParser : public ResourceParser {

		public:
			ScriptParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
		private:
			RID createScript(const char* name, const ScriptDescriptor& descriptor);
		};

	}

}