#include "ScriptParser.h"

namespace ds {

	namespace res {

		// ------------------------------------------------------
		// create script
		// ------------------------------------------------------
		RID ScriptParser::createScript(const char* name, const ScriptDescriptor& descriptor) {
			char fileName[256];
			sprintf_s(fileName, 256, "content\\scripts\\%s.script", descriptor.fileName);
			LOG << "loading script: " << fileName;
			vm::Script* ctx = new vm::Script(fileName);
			if (descriptor.variables != 0) {
				int positions[16];
				int nr = string::count_delimiters(descriptor.variables, positions, 16, ',');
				if (nr > 0) {
					char name[32];
					int s = 0;
					for (int i = 0; i < nr; ++i) {
						int l = positions[i] - s;
						strncpy(name, descriptor.variables + s, l);
						name[l] = '\0';
						ctx->registerVar(SID(name));
						s = positions[i] + 1;
					}
					int l = strlen(descriptor.variables) - s;
					strncpy(name, descriptor.variables + s, l);
					name[l] = '\0';
					ctx->registerVar(SID(name));
				}
				else {
					ctx->registerVar(SID(descriptor.variables));
				}
			}
			if (ctx->load()) {
				int idx = _resCtx->resources.size();
				ScriptResource* cbr = new ScriptResource(ctx);
				_resCtx->resources.push_back(cbr);
				return create(name, ResourceType::SCRIPT);
			}
			else {
				delete ctx;
				return INVALID_RID;
			}
		}

		RID ScriptParser::parse(JSONReader& reader, int childIndex) {
			ScriptDescriptor descriptor;
			descriptor.variables = reader.get_string(childIndex, "variables");
			descriptor.fileName = reader.get_string(childIndex, "file");
			const char* name = reader.get_string(childIndex, "name");
			return createScript(name, descriptor);
		}
	}
}