#pragma once
#include "..\lib\collection_types.h"
#include "..\renderer\render_types.h"
#include "..\sprites\Sprite.h"
#include "..\math\tweening.h"
#include "..\renderer\BitmapFont.h"
#include "..\imgui\IMGUI.h"
#include "..\utils\GameTimer.h"
#include "..\io\DataFile.h"
#include "..\resources\ResourceDescriptors.h"
#include "..\renderer\sprites.h"

namespace ds {

const int MAX_GUID = 64;

// -------------------------------------------------------
// GUI item types
// -------------------------------------------------------
enum GUIItemType {
	GIT_BUTTON,
	GIT_TEXT,
	GIT_IMAGE,
	GIT_IMAGE_BUTTON,
	GIT_TIMER,
	GIT_NUMBERS,
	GIT_UNUSED
};

// -------------------------------------------------------
// GUID
// -------------------------------------------------------
struct GUID {
	int id;
	int index;

	GUID() : id(-1), index(-1) {}
	GUID(int _id, int _index, int _entryIndex) : id(_id), index(_index) {}
};

// -------------------------------------------------------
// Dialog item
// -------------------------------------------------------
struct DialogItem {

	ID id;
	v2 pos;
	bool centered;
	Color color;
	v2 scale;
	float rotation;
	Rect boundingRect;
	GUIItemType type;
	uint32_t index;
	uint32_t num;
	bool active;
	int tmp;
};

// -------------------------------------------------------
// Dialog vertex
// -------------------------------------------------------
struct DialogVertex {

	v2 offset;
	Texture texture;

};


// -------------------------------------------------------
// GUI transitions
// -------------------------------------------------------
struct GUITransition {

	bool active;
	float timer;
	float ttl;
	v2 start;
	v2 end;
	tweening::TweeningType tweening;

	GUITransition() : active(false), timer(0.0f), ttl(0.0f), start(0, 0), end(0, 0), tweening(tweening::easeInCubic) {}
};

// -------------------------------------------------------
// GUI Dialog
// -------------------------------------------------------
class GUIDialog : public DataFile {

	typedef Array<GameTimer> Timers;

public:
	GUIDialog(const GUIDialogDescriptor& descriptor);
	~GUIDialog(void);
	void render();
	void setButtonHover(const Rect& regularItem,const Rect& highlightItem);
	

	GUID addImage(int id,int x,int y,const Rect& textureRect,const v2& scale = v2(1,1),bool centered = true);	
	void updateImage(int id, int x, int y, const Rect& textureRect, bool centered = true);

	GUID addImageButton(int id,int x,int y,const Rect& textureRect,bool centered = true);

	// FIXME: add button with x and y position
	GUID addButton(int id,float x,float y,const char* text,const Rect& textureRect,const Color& textColor = Color(1.0f,1.0f,1.0f,1.0f), const v2& scale = v2(1, 1),bool centered = true);
	void setButtonTexture(int id,const Rect& textureRect);

	GUID addText(int id,int x,int y,const char* text,const Color& color = Color(1.0f,1.0f,1.0f,1.0f), const v2& scale = v2(1, 1),bool centered = true);
	void updateText(int id,int x,int y,const char* text,const Color& color = Color(1.0f,1.0f,1.0f,1.0f), const v2& scale = v2(1, 1),bool centered = true);
	void updateText(int id,const char* text);
	

	GUID addTimer(int id,int x, int y, const v2& scale = v2(1, 1), const Color& color = Color::WHITE, bool centered = true);
	void resetTimer(int id);
	void startTimer(int id);
	GameTimer* getTimer(int id);

	virtual int onButton(int button,int x,int y,bool down);

	virtual void activate();
	virtual void deactivate();		
	const IdString& getHashName() const {
		return m_HashName;
	}
	void updateMousePos(const Vector2f& mousePos);
	void clear();

	void tick(float dt);

	GUID addNumber(int id,const v2& position,int value, int length, const v2& scale = v2(1, 1),const Color& color = Color::WHITE,bool centered = false);
	void setNumber(int id, int value);

	bool saveData(JSONWriter& writer);
	bool loadData(const JSONReader& reader);
	const char* getFileName() const {
		return _jsonName;
	}
	const char* getName() const {
		return _name;
	}

	void startTransition(int id, const v2& start, float ttl);

private:
	void updateTextVertices(int offset, const char* text, int sx = 0, int sy = 0);
	int addTextVertices(const char* text, int sx, int sy);
	GUIDialog(const GUIDialog& other) {}
	int loadItem(int category, const JSONReader& reader, DialogItem* item);
	int findFreeID();
	bool containsItem(int id);
	int getNextID();
	bool swap(int currentIndex, int newIndex);
	int getIndexByID(int id);
	bool remove(int id);
	v2 getPosition(int index);

	IdString m_HashName;
	RID _font;
	Bitmapfont* _bitmapFont;

	SpriteBuffer* _sprites;

	Rect m_ButtonItem;
	Rect m_ButtonItemSelected;
	int m_SelectedInput;

	char _name[32];
	char _jsonName[128];

	GUID _ids[MAX_GUID];
	int _idIndex;
	GUITransition _transitions[MAX_GUID];	

	Array<DialogVertex> _vertices;
	Array<DialogItem> _items;
	Timers _timers;
};

}
