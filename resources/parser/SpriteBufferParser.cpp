#include "SpriteBufferParser.h"

namespace ds {

	namespace res {

		RID SpriteBufferParser::createSpriteBuffer(const char* name, const SpriteBufferDescriptor& descriptor) {
			SpriteBuffer* buffer = new SpriteBuffer(descriptor);
			SpriteBufferResource* cbr = new SpriteBufferResource(buffer);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::SPRITEBUFFER);
		}

		RID SpriteBufferParser::parse(JSONReader& reader, int childIndex) {
			SpriteBufferDescriptor descriptor;
			reader.get(childIndex, "size", &descriptor.size);
			reader.get(childIndex, "index_buffer", &descriptor.indexBuffer);
			const char* constantBufferName = reader.get_string(childIndex, "constant_buffer");
			descriptor.constantBuffer = find(constantBufferName, ResourceType::CONSTANTBUFFER);
			const char* vertexBufferName = reader.get_string(childIndex, "vertex_buffer");
			descriptor.vertexBuffer = find(vertexBufferName, ResourceType::VERTEXBUFFER);
			const char* materialName = reader.get_string(childIndex, "material");
			descriptor.material = find(materialName, ResourceType::MATERIAL);
			if (reader.contains_property(childIndex, "font")) {
				const char* fontName = reader.get_string(childIndex, "font");
				descriptor.font = find(fontName, ResourceType::BITMAPFONT);
			}
			else {
				descriptor.font = INVALID_RID;
			}
			const char* name = reader.get_string(childIndex, "name");
			return createSpriteBuffer(name, descriptor);
		}
	}
}