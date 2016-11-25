#include "SquareBufferParser.h"

namespace ds {

	namespace res {

		RID SquareBufferParser::createSquareBuffer(const char* name, int size, RID texture) {
			SquareBuffer* buffer = graphics::createSquareBuffer(name, size, texture);
			SquareBufferResource* cbr = new SquareBufferResource(buffer);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::SQUAREBUFFER);
		}

		RID SquareBufferParser::parse(JSONReader& reader, int childIndex) {
			uint32_t size = 0;
			reader.get(childIndex, "size", &size);
			const char* textureName = reader.get_string(childIndex, "texture");
			RID texture = find(textureName, ResourceType::TEXTURE);
			const char* name = reader.get_string(childIndex, "name");
			return createSquareBuffer(name, size, texture);
		}
	}
}