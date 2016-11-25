#include "ResourceParser.h"

namespace ds {

	namespace res {

		RID ResourceParser::create(const char* name, ResourceType type) {
			StaticHash hash = StaticHash(name);
			ResourceIndex ri;
			// this one gets called after we have already the resource
			ri.id = _resCtx->resources.size() - 1;
			ri.nameIndex = _resCtx->nameBuffer.size;
			_resCtx->nameBuffer.append(name);
			ri.type = type;
			ri.hash = hash;
			LOG << "adding resource: '" << name << "' at " << ri.id << " type: " << ResourceTypeNames[type];
			_resCtx->indices.push_back(ri);
			return ri.id;
		}
	}
}