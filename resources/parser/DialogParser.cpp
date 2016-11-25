#include "DialogParser.h"

namespace ds {

	namespace res {

		RID  DialogParser::createDialog(const char* name, const GUIDialogDescriptor& descriptor) {
			char fileName[256];
			sprintf_s(fileName, 256, "content\\dialogs\\%s.json", descriptor.file);
			LOG << "loading dialog: " << fileName;
			GUIDialog* dialog = new GUIDialog(descriptor, fileName);
			if (dialog->load()) {
				int idx = _resCtx->resources.size();
				GUIDialogResource* cbr = new GUIDialogResource(dialog);
				_resCtx->resources.push_back(cbr);
				return create(name, ResourceType::GUIDIALOG);
			}
			else {
				delete dialog;
				return INVALID_RID;
			}
		}

		RID DialogParser::parse(JSONReader& reader, int childIndex) {
			GUIDialogDescriptor descriptor;
			const char* fontName = reader.get_string(childIndex, "font");
			descriptor.font = find(fontName, ResourceType::BITMAPFONT);
			descriptor.file = reader.get_string(childIndex, "file");
			const char* name = reader.get_string(childIndex, "name");
			return createDialog(name, descriptor);
		}
	}
}