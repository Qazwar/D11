#include "WorldEntityTemplatesParser.h"

namespace ds {

	namespace res {

		RID WorldEntityTemplatesParser::loadWorldEntityTemplates(const char* name) {
			char buffer[256];
			sprintf_s(buffer, 256, "content\\resources\\%s.json", name);
			WorldEntityTemplates* sheet = new WorldEntityTemplates(buffer);
			sheet->load();
			WorldEntityTemplatesResource* cbr = new WorldEntityTemplatesResource(sheet);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::ENTITY_TEMPLATES);
		}

		RID WorldEntityTemplatesParser::parse(JSONReader& reader, int childIndex) {
			const char* name = reader.get_string(childIndex, "name");
			return loadWorldEntityTemplates(name);
		}
	}
}