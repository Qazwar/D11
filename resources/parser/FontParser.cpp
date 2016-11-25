#include "FontParser.h"

namespace ds {

	namespace res {

		RID FontParser::loadFont(const char* name, const BitmapfontDescriptor& descriptor) {
			char buffer[256];
			sprintf_s(buffer, 256, "content\\resources\\%s", descriptor.name);
			float xOffset = 0.0f;
			float yOffset = 0.0f;
			float textureSize = 1024.0f;
			Bitmapfont* font = new Bitmapfont;
			JSONReader reader;
			bool ret = reader.parse(SID(buffer));
			assert(ret);
			int flipped = 0;
			int info = reader.find_category("settings");
			if (info != -1) {
				reader.get_float(info, "x_offset", &xOffset);
				reader.get_float(info, "y_offset", &yOffset);
				reader.get_float(info, "texture_size", &textureSize);
				reader.get_int(info, "axis_flipped", &flipped);
			}
			int num = reader.find_category("characters");
			char tmp[16];
			Rect rect;
			if (num != -1) {
				for (int i = 32; i < 255; ++i) {
					sprintf_s(tmp, 16, "C%d", i);
					if (reader.contains_property(num, tmp)) {
						reader.get(num, tmp, &rect);
						if (flipped == 1) {
							float w = rect.width();
							float h = rect.height();
							float tm = rect.top;
							rect.top = rect.left;
							rect.left = tm;
							rect.right = rect.left + w;
							rect.bottom = rect.top + h;
						}
						font->add(i, rect, xOffset, yOffset, textureSize);
					}
				}
			}
			BitmapfontResource* cbr = new BitmapfontResource(font);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::BITMAPFONT);
		}

		RID FontParser::parse(JSONReader& reader, int childIndex) {
			BitmapfontDescriptor descriptor;
			descriptor.name = reader.get_string(childIndex, "file");
			const char* name = reader.get_string(childIndex, "name");
			return loadFont(name, descriptor);
		}
	}
}