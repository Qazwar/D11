#include "MaterialParser.h"

namespace ds {

	namespace res {

		RID MaterialParser::createMaterial(const char* name, const MaterialDescriptor& descriptor) {
			Material* m = new Material;
			m->ambient = descriptor.ambient;
			m->diffuse = descriptor.diffuse;
			m->shader = descriptor.shader;
			m->texture = descriptor.texture;
			m->renderTarget = descriptor.renderTarget;
			MaterialResource* cbr = new MaterialResource(m);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::MATERIAL);
		}

		RID MaterialParser::parse(JSONReader& reader, int childIndex) {
			MaterialDescriptor descriptor;
			const char* shaderName = reader.get_string(childIndex, "shader");
			descriptor.shader = find(shaderName, ResourceType::SHADER);
			const char* bsName = reader.get_string(childIndex, "blend_state");
			descriptor.blendstate = find(bsName, ResourceType::BLENDSTATE);
			const char* name = reader.get_string(childIndex, "name");
			reader.get(childIndex, "diffuse", descriptor.diffuse);
			reader.get(childIndex, "ambient", descriptor.ambient);
			if (reader.contains_property(childIndex, "texture")) {
				const char* texName = reader.get_string(childIndex, "texture");
				descriptor.texture = find(texName, ResourceType::TEXTURE);
			}
			else {
				descriptor.texture = INVALID_RID;
			}
			if (reader.contains_property(childIndex, "render_target")) {
				const char* texName = reader.get_string(childIndex, "render_target");
				descriptor.renderTarget = find(texName, ResourceType::RENDERTARGET);
			}
			else {
				descriptor.renderTarget = INVALID_RID;
			}
			return createMaterial(name, descriptor);
		}
	}
}