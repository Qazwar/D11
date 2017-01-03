#include "SceneParser.h"

namespace ds {

	namespace res {

		RID SceneParser::parse(JSONReader& reader, int childIndex) {
			SceneDescriptor descriptor;
			reader.get(childIndex, "size", &descriptor.size);
			descriptor.meshBuffer = reader.get_string(childIndex, "mesh_buffer");
			descriptor.camera = reader.get_string(childIndex, "camera");
			reader.get(childIndex, "depth_enabled", &descriptor.depthEnabled);
			const char* name = reader.get_string(childIndex, "name");
			Scene* scene = new Scene(descriptor);
			int idx = _resCtx->resources.size();
			SceneResource* cbr = new SceneResource(scene);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::SCENE);
		}
	}
}