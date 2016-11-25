#include "SoundParser.h"
#include "..\..\audio\AudioManager.h"

namespace ds {

	namespace res {

		RID SoundParser::parse(JSONReader& reader, int childIndex) {
			SoundDescriptor descriptor;
			descriptor.fileName = reader.get_string(childIndex, "file");
			descriptor.name = reader.get_string(childIndex, "name");
			char buffer[256];
			sprintf_s(buffer, 256, "content\\sounds\\%s", descriptor.fileName);
			int snd = audio::load(SID(descriptor.name), SID(buffer));
			return INVALID_ID;
		}
	}
}