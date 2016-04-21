#pragma once
#include "GUIDialog.h"
#include "..\io\DataFile.h"
#include "..\imgui\IMGUI.h"

namespace ds {

struct DialogDefinition {
	char name[32];
	IdString hash;
	GUIDialog* dialog;

};

typedef Array<DialogDefinition> Dialogs;

class DialogManager : public DataFile {

public:
	DialogManager(const DialogManagerDescriptor& descriptor);
	~DialogManager(void);	
	void render();
	void activate(const char* dialogName);
	void deactivate(const char* dialogName);
	bool onButtonUp(int button,int x,int y,DialogID* dlgId,int* selected);
	void updateMousePos(const Vector2f& mousePos);
	GUIDialog* get(const char* dialogName);
	GUIDialog* create(const char* dialogName);
	void tick(float dt);
	bool saveData(JSONWriter& writer);
	const char* getFileName() const {
		return "resources\\gui.json";
	}
	const Dialogs& getDialogs() const {
		return _dialogs;
	}
	bool remove(const char* name);
	bool contains(const char* name) const;
	virtual bool loadData(const JSONReader& loader);
private:		
	void clear();
	void setActiveFlag(const char* name,bool active);	
	void createDialog(const char* name,int id,GUIDialog* dialog);
	Bitmapfont* _font;
	bool _initialized;
	Dialogs _dialogs;
	uint32_t _index;
	int _current;
};

}

