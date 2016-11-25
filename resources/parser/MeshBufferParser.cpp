#include "MeshBufferParser.h"

namespace ds {

	namespace res {

		RID MeshBufferParser::createMeshBuffer(const char* name, const MeshBufferDescriptor& descriptor) {
			MeshBuffer* buffer = new MeshBuffer(descriptor);
			MeshBufferResource* cbr = new MeshBufferResource(buffer);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::MESHBUFFER);
		}

		RID MeshBufferParser::parse(JSONReader& reader, int childIndex) {
			MeshBufferDescriptor descriptor;
			reader.get(childIndex, "size", &descriptor.size);
			const char* indexBufferName = reader.get_string(childIndex, "index_buffer");
			descriptor.indexBuffer = find(indexBufferName, ResourceType::INDEXBUFFER);
			const char* constantBufferName = reader.get_string(childIndex, "constant_buffer");
			descriptor.constantBuffer = find(constantBufferName, ResourceType::CONSTANTBUFFER);
			const char* vertexBufferName = reader.get_string(childIndex, "vertex_buffer");
			descriptor.vertexBuffer = find(vertexBufferName, ResourceType::VERTEXBUFFER);
			const char* materialName = reader.get_string(childIndex, "material");
			descriptor.material = find(materialName, ResourceType::MATERIAL);
			const char* name = reader.get_string(childIndex, "name");
			return createMeshBuffer(name, descriptor);
		}
	}
}