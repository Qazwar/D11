#include "IMGUIParser.h"

namespace ds {

	namespace res {

		RID IMGUIParser::parse(JSONReader& reader, int childIndex) {
			IMGUIDescriptor descriptor;
			const char* fontName = reader.get_string(childIndex, "font");
			descriptor.font = find(fontName, ResourceType::BITMAPFONT);
			const char* texName = reader.get_string(childIndex, "texture");
			descriptor.texture = find(texName, ResourceType::TEXTURE);
			gui::initialize(descriptor);
			return INVALID_RID;
		}
	}
}