#include "MeshParser.h"

namespace ds {

	namespace res {

		RID MeshParser::createMesh(const char* name, const MeshDescriptor& descriptor) {
			Mesh* mesh = new Mesh;
			mesh->load(descriptor.fileName);
			MeshResource* cbr = new MeshResource(mesh);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::MESH);
		}

		RID MeshParser::parse(JSONReader& reader, int childIndex) {
			MeshDescriptor descriptor;
			reader.get(childIndex, "position", &descriptor.position);
			reader.get(childIndex, "scale", &descriptor.scale);
			reader.get(childIndex, "rotation", &descriptor.rotation);
			const char* name = reader.get_string(childIndex, "name");
			descriptor.fileName = name;
			return createMesh(name, descriptor);
		}
	}
}