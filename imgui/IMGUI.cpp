#include "IMGUI.h"
#include "..\math\math.h"
#include "..\graphics.h"
#include "..\renderer\sprites.h"
#include "..\utils\Profiler.h"
#include "..\utils\Log.h"
#include <stdarg.h>

namespace gui {

	const float BUTTON_HEIGHT = 24.0f;
	const float TEXTFIELD_HEIGHT = 16.0f;
	const float TEXT_PADDING = 4.0f;
	const float BUTTON_PADDING = 20.0f;
	//const float LINE_HEIGHT = 26.0f;
	const float TEXT_OFFSET = 7.0f;
	const int CHAR_PADDING = 0;
	const float BOX_HEIGHT = 18.0f;
	const float WHITE_BOX_SIZE = 256.0f;
	const float INPUT_BOX_WIDTH = 70.0f;
	//const float LABEL_SIZE = 80.0f;
	const char* EMPTY_SELECTION = "Please select one";

	struct SettingBackup {
		GUISetting setting;
		float value;
	};

	enum TilingDef {
		TD_NONE,
		TD_TILE_X,
		TD_TILE_BOTH,
		TD_TILE_Y,
		TD_STRETCH
	};

	typedef uint32_t HashedId;

	static HashedId NULL_HASH = 0;

	HashedId HashId(const char *id) {
		// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
		HashedId hash = 0x84222325;
		while (*id) hash = (hash ^ static_cast<uint8_t>(*id++)) * 0x000001b3;
		assert(hash != NULL_HASH);
		return hash;
	}

	HashedId HashPointer(const void *ptr) {
		HashedId hash = static_cast<HashedId>(reinterpret_cast<size_t>(ptr)) * 2654435761;
		assert(hash != NULL_HASH);
		return hash;
	}

	bool getClipboardText(char* buffer,int max) {
		// Try opening the clipboard
		if (!OpenClipboard(nullptr)) {
			return false;
		}
			

		// Get handle of clipboard object for ANSI text
		HANDLE hData = GetClipboardData(CF_TEXT);
		if (hData == nullptr) {
			return false;
		}
		// Lock the handle to get the actual text pointer
		char * pszText = static_cast<char*>(GlobalLock(hData));
		if (pszText == nullptr) {
			return false;
		}
		// Save text in a string class instance
		sprintf_s(buffer, max, pszText);

		// Release the lock
		GlobalUnlock(hData);

		// Release the clipboard
		CloseClipboard();

		return true;
	}

	enum DrawCallType {
		DCT_BOX,
		DCT_IMAGE,
		DCT_TEXT,
		DCT_HEADER,
		DCT_EXTERNAL_IMAGE,
		DCT_EOL
	};
	// -------------------------------------------------------
	// DrawCall
	// -------------------------------------------------------
	struct DrawCall {

		DrawCallType type;
		int textIndex;
		ds::Color color;
		v2 size;
		v2 position;
		ds::Texture texture;
		int padding;
		TilingDef tilingDef;
		float additional;
	};

	const int MAX_DRAW_CALLS = 256;
	// -------------------------------------------------------
	// GUIWindow
	// -------------------------------------------------------
	struct GUIWindow {

		ds::Array<DrawCall> calls;
		ds::CharBuffer textBuffer;

		void reset() {
			calls.clear();
			textBuffer.size = 0;
		}

		void addBox(const v2& position,const v2& size, const ds::Color& color, bool stretch = false) {
			DrawCall call;
			call.type = DCT_BOX;
			call.color = color;
			call.size = size;
			call.position = position;
			call.padding = 2;
			call.tilingDef = TD_NONE;
			if (stretch) {
				call.tilingDef = TD_STRETCH;
			}
			calls.push_back(call);
		}

		void addTiledXImage(const v2& position, const ds::Texture& texture, float width,float cornerSize) {
			DrawCall call;
			call.type = DCT_IMAGE;
			call.color = ds::Color::WHITE;
			call.size = v2(width,1.0f);
			call.position = position;
			call.texture = texture;
			call.padding = 1;
			call.additional = cornerSize;
			call.tilingDef = TD_TILE_X;
			calls.push_back(call);
		}

		void addTiledXYImage(const v2& position, const ds::Texture& texture, const v2& size,float cornerSize) {
			DrawCall call;
			call.type = DCT_IMAGE;
			call.color = ds::Color::WHITE;
			call.size = size;
			call.position = position;
			call.texture = texture;
			call.padding = 1;
			call.additional = cornerSize;
			call.tilingDef = TD_TILE_BOTH;
			calls.push_back(call);
		}

		void addImage(const v2& position, const ds::Texture& texture,const v2& size = v2(1,1)) {
			DrawCall call;
			call.type = DCT_IMAGE;
			call.color = ds::Color::WHITE;
			call.size = size;
			call.position = position;
			call.texture = texture;
			call.padding = 1;
			call.tilingDef = TD_NONE;
			calls.push_back(call);
		}

		void addExternalImage(const v2& position, const ds::Texture& texture, int textureID,const ds::Color& clr = ds::Color::WHITE) {
			DrawCall call;
			call.type = DCT_EXTERNAL_IMAGE;
			call.color = clr;
			call.size = texture.dim;
			call.position = position;
			call.texture = texture;
			call.padding = 1;
			call.additional = textureID;
			call.tilingDef = TD_NONE;
			calls.push_back(call);
		}

		void addText(const v2& position,const char* text, const v2& size, float textOffset = TEXT_OFFSET) {
			DrawCall call;
			call.type = DCT_TEXT;
			call.color = ds::Color::WHITE;
			call.size = size;
			call.textIndex = textBuffer.size;
			textBuffer.append(text);
			call.position = position;
			call.position.y -= textOffset;
			call.padding = CHAR_PADDING;
			call.tilingDef = TD_NONE;
			calls.push_back(call);
		}

		void addHeader(const v2& position, const char* text, const v2& size) {
			DrawCall call;
			call.type = DCT_HEADER;
			call.color = ds::Color::WHITE;
			call.size = size;
			call.textIndex = textBuffer.size;
			textBuffer.append(text);
			call.position = position;
			call.position.y -= TEXT_OFFSET;
			call.padding = CHAR_PADDING;
			call.tilingDef = TD_NONE;
			calls.push_back(call);
		}

		
	};

	// -------------------------------------------------------
	// KeyInput
	// -------------------------------------------------------
	struct KeyInput {
		unsigned char keys[256];
		int num;
	};

	enum GUIDataType {
		DT_INT,
		DT_FLOAT
	};

	enum GUIIcon {
		ICN_ARROW_LEFT,
		ICN_ARROW_DOWN,
		ICN_ARROW_RIGHT,
		ICN_ARROW_UP,
		ICN_PLUS,
		ICN_MINUS,
		ICN_CHECKBOX,
		ICN_CHECKBOX_SELECTED,
		ICN_DRAG_BOX,
		ICN_BUTTON,
		ICN_INPUT,
		ICN_INPUT_ACTIVE,
		ICN_HEADER_BOX,
		ICN_PANEL_BACKGROUND,
		ICN_BOX_BACKGROUND,
		ICN_SEPARATOR,
		ICN_STEP_INPUT,
		ICN_STEP_INPUT_SMALL,
		ICN_COLOR_SLIDER
	};

	enum GUIColor {
		CLR_PANEL_BACKGROUND,
		CLR_PANEL_HEADER,
		CLR_INPUT_EDIT,
		CLR_INPUT,
		CLR_BUTTON,
		CLR_BUTTON_HOVER,
		CLR_SELECTED_LINE,
		CLR_SLIDER,
		CLR_PROGRESS,
		CLR_IMAGE_BACKGROUND
	};

	// -------------------------------------------------------
	// GUI context
	// -------------------------------------------------------
	struct GUIContext {

		ds::Bitmapfont* font;
		int textureID;
		GUIWindow window;
		v2 cursorPosition;
		bool buttonPressed;
		bool clicked;
		bool released;
		HashedId hot;
		HashedId active;
		bool grouped;
		char inputText[32];
		KeyInput keyInput;
		bool visible;
		int caretPos;
		v2 position;
		const char* header;
		bool useHeader;
		v2 startPosition;
		ds::Texture textures[32];
		ds::Color colors[16];
		bool ready;
		int dragging;
		bool started;
		char tempBuffer[64];
		bool editorMode;
		bool modal;
		float settings[32];
		float backup[32];
		ds::SpriteBuffer* spriteBuffer;

		GUIContext() {
			textureID = -1;
			ready = false;
			editorMode = false;
			keyInput.num = 0;
			modal = false;
			useHeader = true;
			settings[GS_LABELSIZE] = 80.0f;
			settings[GS_LINE_HEIGHT] = 26.0f;
		}

		void reset() {
			cursorPosition = graphics::getMousePosition();
			//clicked = false;
			grouped = false;
			visible = false;
			window.reset();
			hot = -1;
			modal = false;
			useHeader = true;
		}

		void addBox(const v2& position, const v2& size, const ds::Color& color, bool stretch = false) {
			v2 p = position;
			p.x += size.x * 0.5f;
			window.addBox(p, size, color, stretch);

		}

		void addTiledXBox(const v2& position, float width,const ds::Texture& texture,float cornerSize) {
			v2 p = position;
			p.x += width * 0.5f;
			window.addTiledXImage(position, texture, width,cornerSize);

		}

		void addTiledXYBox(const v2& position, const v2& size, const ds::Texture& texture,float cornerSize) {
			v2 p = position;
			p.x += size.x * 0.5f;
			window.addTiledXYImage(position, texture, size, cornerSize);

		}
		
		void addImage(const v2& position, const ds::Texture& texture,float offsetX = 0.0f,float offsetY = 0.0f) {
			v2 p = position;
			p.x += offsetX;
			p.y += offsetY;
			window.addImage(p, texture);
		}

		void addExternalImage(const v2& position, const ds::Texture& texture, int textureID, const ds::Color& clr) {
			v2 p = position;
			window.addExternalImage(p, texture, textureID, clr);
		}

		void addText(const v2& position, const char* text) {
			v2 size = ds::font::calculateSize(*font, text, CHAR_PADDING);
			v2 p = position;
			p.x += TEXT_PADDING;
			addText(p, text, size);
		}

		void addHeader(const v2& position, const char* text) {
			v2 size = ds::font::calculateSize(*font, text, CHAR_PADDING);
			v2 p = position;
			p.x += TEXT_PADDING;
			window.addHeader(position, text, size);
		}

		void addText(const v2& position, const char* text, const v2& size, float textOffset = TEXT_OFFSET) {
			window.addText(position, text, size, textOffset);
		}

		void nextPosition() {
			nextPosition(settings[GS_LINE_HEIGHT]);			
		}

		void nextPosition(float height) {
			if (!grouped) {
				position.y -= height;
				position.x = startPosition.x;				
			}
			else {
				position.x += window.calls[window.calls.size() - 1].size.x + 50.0f;
				
			}
		}
	};

	static GUIContext* guiContext = 0;

	void switchEditorMode(bool editorMode) {
		guiContext->editorMode = editorMode;
	}

	// -------------------------------------------------------
	// send key
	// -------------------------------------------------------
	void sendKey(unsigned char c) {
		if (guiContext->ready && guiContext->active != -1) {			
			LOG << "received key: " << c << " (" << (int)c << ")";
			if (guiContext->keyInput.num < 256) {
				guiContext->keyInput.keys[guiContext->keyInput.num++] = c;
			}
			else {
				guiContext->keyInput.num = 0;
			}
		}
	}
	
	// -------------------------------------------------------
	// send special key
	// -------------------------------------------------------
	void sendSpecialKey(WPARAM wParam) {
		switch (wParam) {
			case VK_BACK   : gui::sendKey(128); break;
			case VK_LEFT   : gui::sendKey(129); break;
			case VK_RIGHT  : gui::sendKey(130); break;
			case VK_HOME   : gui::sendKey(131); break;
			case VK_END    : gui::sendKey(132); break;
			case VK_RETURN : gui::sendKey(133); break;
			case VK_DELETE : gui::sendKey(134); break;
		}
	}
	// -------------------------------------------------------
	// check if mouse cursor is inside box
	// -------------------------------------------------------
	bool isCursorInside(const v2& p, const v2& dim) {
		v2 mp = guiContext->cursorPosition;
		if (mp.x < (p.x - dim.x * 0.5f)) {
			return false;
		}
		if (mp.x > (p.x + dim.x * 0.5f)) {
			return false;
		}
		if (mp.y < (p.y - dim.y * 0.5f)) {
			return false;
		}
		if (mp.y > (p.y + dim.y * 0.5f)) {
			return false;
		}
		return true;
	}

	ds::Texture buildBoxTexture(const v2& size) {
		return math::buildTexture(200.0f, 0.0f, size.x, size.y);
	}

	ds::Texture buildBoxTexture(float dimX,float dimY) {
		return math::buildTexture(200.0f, 0.0f, dimX, dimY);
	}

	// -------------------------------------------------------
	// begin group
	// -------------------------------------------------------
	void beginGroup() {
		guiContext->grouped = true;
	}

	// -------------------------------------------------------
	// end group
	// -------------------------------------------------------
	void endGroup() {
		guiContext->grouped = false;
		guiContext->nextPosition(guiContext->settings[GS_LINE_HEIGHT] + 4.0f);
	}

	// -------------------------------------------------------
	// check if widget is hot
	// -------------------------------------------------------
	bool isHot(HashedId id, const v2& pos, const v2& size, float offsetX = 0.0f, float offsetY = 0.0f) {
		v2 p = pos;
		p.x += offsetX;
		p.y += offsetY;		
		if (isCursorInside(p, size)) {
			guiContext->hot = id;
			return true;
		}
		return false;
	}

	// -------------------------------------------------------
	// handle mouse interaction
	// -------------------------------------------------------
	bool isSelected(HashedId id, const v2& pos, const v2& size, bool setActive = true) {
		if (guiContext->clicked && isCursorInside(pos, size)) {
			if (setActive) {
				guiContext->active = id;
			}
			return true;
		}		
		return false;
	}

	// -------------------------------------------------------
	// handle mouse interaction
	// -------------------------------------------------------
	bool isClicked(const v2& pos, const v2& size) {
		if (guiContext->clicked && isCursorInside(pos, size)) {
			return true;
		}
		return false;
	}

	// -------------------------------------------------------
	// is box selected
	// -------------------------------------------------------
	bool isBoxSelected(HashedId id, const v2& pos, const v2& size, bool setActive = true) {
		if (guiContext->clicked) {
			v2 p = pos;
			p.x += size.x * 0.5f;
			if (guiContext->clicked && isCursorInside(p, size)) {
				if (setActive) {
					guiContext->active = id;
					guiContext->clicked = false;
				}
				return true;
			}
		}
		return false;
	}

	// -------------------------------------------------------
	// is box selected
	// -------------------------------------------------------
	bool isDragging(const v2& pos, const v2& size) {
		if (guiContext->buttonPressed) {
			v2 p = pos;
			p.x += size.x * 0.5f;
			return isCursorInside(p, size);
		}
		return false;
	}

	void endFrame() {
		if (guiContext != 0 && guiContext->ready) {
			guiContext->started = false;
		}
	}
	// -------------------------------------------------------
	// start
	// -------------------------------------------------------
	void start(int id,v2* startPos,bool forcePosition) {
		assert(guiContext->ready);		
		if (!guiContext->started) {
			guiContext->started = true;
			if (guiContext->hot == -1 && guiContext->clicked) {
				guiContext->active = -1;
			}
			if (guiContext->clicked) {
				guiContext->clicked = false;
			}
			guiContext->hot = -1;

			if ((GetKeyState(VK_LBUTTON) & 0x80) != 0) {
				guiContext->buttonPressed = true;
			}
			else {
				if (guiContext->buttonPressed) {
					if (guiContext->dragging == -1) {
						guiContext->clicked = true;
					}
					else {
						guiContext->dragging = -1;
					}
				}
				guiContext->buttonPressed = false;
			}
			if (guiContext->editorMode) {
				guiContext->startPosition = v2(1050,750);
				guiContext->position = v2(1050, 750);
			}
			else {
				guiContext->startPosition = *startPos;
				guiContext->position = *startPos;
			}
		}		
		if (!forcePosition) {
			v2 dragBoxPos = *startPos;
			dragBoxPos.x += 100.0f;
			if (guiContext->dragging == -1 && guiContext->buttonPressed && isCursorInside(dragBoxPos, v2(200.0f, BOX_HEIGHT))) {
				guiContext->dragging = id;
			}
			if (guiContext->dragging == id) {
				v2 correctPos = guiContext->cursorPosition;
				correctPos.x -= 100.0f;
				guiContext->startPosition = correctPos;
				*startPos = correctPos;
			}
		}
		if (forcePosition) {
			guiContext->startPosition = *startPos;
			guiContext->position = *startPos;
		}
	}

	// -------------------------------------------------------
	// begin panel
	// -------------------------------------------------------
	void begin() {
		guiContext->reset();
		guiContext->useHeader = false;
		guiContext->visible = true;
		guiContext->dragging = -1;
		guiContext->nextPosition();
	}

	// -------------------------------------------------------
	// begin panel
	// -------------------------------------------------------
	bool begin(const char* header,int* state) {
		guiContext->reset();
		guiContext->header = header;
		
		// build panel header
		float width = 200.0f;
		v2 p = guiContext->position;
		guiContext->startPosition = p;
		bool active = isBoxSelected(0, p, v2(width, BOX_HEIGHT));
		if (active) {
			if (*state == 0) {
				*state = 1;
			}
			else {
				*state = 0;
			}
		}		
		if (*state == 1) {
			guiContext->visible = true;
		}
		else {
			guiContext->visible = false;
		}
		guiContext->nextPosition();		
		return *state == 1;
	}

	// -------------------------------------------------------
	// begin modal panel
	// -------------------------------------------------------
	void beginModal(const char* header) {
		guiContext->reset();
		guiContext->header = header;
		// FIXME: screen center
		//guiContext->startPosition = v2(512,384);
		//guiContext->position = v2(512, 384);
		guiContext->visible = true;
		//guiContext->nextPosition();
		guiContext->modal = true;
	}

	v2 getTextSize(const char* text) {
		return ds::font::calculateSize(*guiContext->font, text,CHAR_PADDING);
	}

	// -------------------------------------------------------
	// Label
	// -------------------------------------------------------
	void Label(const char* text) {
		v2 p = guiContext->position;
		HashedId id = HashId(text);
		bool hot = isHot(id,p, getTextSize(text));
		guiContext->addText(p, text);
		guiContext->nextPosition();
	}

	void Label(const char* text, const char* fmt, ...) {
		v2 p = guiContext->position;
		HashedId id = HashId(text);
		bool hot = isHot(id, p, getTextSize(text));
		guiContext->addText(p, text);
		p.x += guiContext->settings[GS_LABELSIZE];
		va_list args;
		va_start(args, fmt);
		vsprintf(guiContext->tempBuffer, fmt, args);
		guiContext->addText(p, guiContext->tempBuffer);
		va_end(args);
		guiContext->nextPosition();
	}

	// -------------------------------------------------------
	// Value
	// -------------------------------------------------------
	void Value(const char* label, int v) {
		Label(label, "%d", v);
	}

	void Value(const char* label, float v) {
		Label(label, "%g", v);
	}

	void Value(const char* label, const v2& v) {
		Label(label, "%g, %g", v.x,v.y);
	}

	void Value(const char* label, const v3& v) {
		Label(label, "%g, %g, %g", v.x, v.y, v.z);
	}

	void Value(const char* label, const ds::Color& v) {
		Label(label, "%g, %g, %g, %g", v.r, v.g, v.b, v.a);
	}

	void Value(const char* label, const char* text) {
		Label(label, "%s", text);
	}

	// -------------------------------------------------------
	// Text
	// -------------------------------------------------------
	void Text(const char* fmt, ...) {
		va_list args;
		va_start(args, fmt);
		vsprintf(guiContext->tempBuffer, fmt, args);
		v2 p = guiContext->position;
		HashedId id = HashId(guiContext->tempBuffer);
		bool hot = isHot(id, p, getTextSize(guiContext->tempBuffer));
		guiContext->addText(p, guiContext->tempBuffer);
		guiContext->nextPosition();
		va_end(args);
	}

	void Header(const char* text) {
		v2 p = guiContext->position;
		HashedId id = HashId(text);
		bool hot = isHot(id, p, getTextSize(text));
		guiContext->addHeader(p, text);
		guiContext->nextPosition(36.0f);
	}

	// -------------------------------------------------------
	// handle text input
	// -------------------------------------------------------
	bool handleTextInput() {
		bool ret = false;
		int len = strlen(guiContext->inputText);
		if (guiContext->keyInput.num > 0) {
			for (int i = 0; i < guiContext->keyInput.num; ++i) {
				if (guiContext->keyInput.keys[i] == 128) {
					if (guiContext->caretPos > 0) {
						if (guiContext->caretPos < len) {
							memmove(guiContext->inputText + guiContext->caretPos - 1, guiContext->inputText + guiContext->caretPos, len - guiContext->caretPos);
						}
						--guiContext->caretPos;
						--len;
						guiContext->inputText[len] = '\0';
						
					}
				}				
				else if (guiContext->keyInput.keys[i] == 129) {
					if (guiContext->caretPos > 0) {
						--guiContext->caretPos;
					}
				}
				else if (guiContext->keyInput.keys[i] == 130) {
					if (guiContext->caretPos < strlen(guiContext->inputText)) {
						++guiContext->caretPos;
					}
				}
				else if (guiContext->keyInput.keys[i] == 131) {
					guiContext->caretPos = 0;
				}
				else if (guiContext->keyInput.keys[i] == 132) {
					guiContext->caretPos = strlen(guiContext->inputText);
				}
				else if (guiContext->keyInput.keys[i] == 133) {
					// return pressed
					guiContext->active = -1;
					ret = true;
				}
				else if (guiContext->keyInput.keys[i] == 22) {
					char buffer[128];
					if (getClipboardText(buffer, 128)) {
						strcpy(guiContext->inputText, buffer);
						len = strlen(guiContext->inputText);
						guiContext->caretPos = len;
					}
				}
				else if (guiContext->keyInput.keys[i] == 134) {
					if (len > 0) {
						if (guiContext->caretPos < len) {							
							memmove(guiContext->inputText + guiContext->caretPos, guiContext->inputText + guiContext->caretPos + 1, len - guiContext->caretPos);
							--len;
							guiContext->inputText[len] = '\0';
						}						
					}
				}
				else if (guiContext->keyInput.keys[i] > 31 && guiContext->keyInput.keys[i] < 128) {
					if (len < 32) {
						if (guiContext->caretPos < len) {
							memmove(guiContext->inputText + guiContext->caretPos + 1, guiContext->inputText + guiContext->caretPos, len - guiContext->caretPos);
						}
						guiContext->inputText[guiContext->caretPos] = guiContext->keyInput.keys[i];
						++len;
						++guiContext->caretPos;
					}
				}
			}
			//++len;
			guiContext->inputText[len] = '\0';
			guiContext->keyInput.num = 0;
		}
		return ret;
	}



	// -------------------------------------------------------
	// input scalar
	// -------------------------------------------------------
	void InputScalar(int id, int index, int* v, float width = INPUT_BOX_WIDTH) {
		ZoneTracker z("IMGUI::InputScalar-I");
		int new_id = id + 1024 * index;
		v2 p = guiContext->position;
		p.x += (width + 10.0f) * index + guiContext->settings[GS_LABELSIZE];
		bool hot = isHot(new_id, p, v2(width, BOX_HEIGHT),width * 0.5f);
		bool selected = isBoxSelected(new_id, p, v2(width, BOX_HEIGHT));
		if (selected) {			
			sprintf_s(guiContext->inputText, 32, "%d", *v);
			guiContext->caretPos = strlen(guiContext->inputText);			
			guiContext->active = new_id;
		}
		if (guiContext->active == new_id) {
			guiContext->addTiledXBox(p, width, guiContext->textures[ICN_INPUT_ACTIVE], BOX_HEIGHT);
			handleTextInput();
			*v = atoi(guiContext->inputText);
			v2 cp = p;
			v2 cursorPos = ds::font::calculateLimitedSize(*guiContext->font, guiContext->inputText,guiContext->caretPos,CHAR_PADDING);
			cp.x = guiContext->position.x + TEXT_PADDING + (width + 10.0f)  * index + cursorPos.x + guiContext->settings[GS_LABELSIZE];
			guiContext->addBox(cp, v2(2, BOX_HEIGHT - 4.0f), ds::Color(192, 0, 0, 255));
			p.y -= 1.0f;
			guiContext->addText(p, guiContext->inputText);
		}
		else {
			sprintf_s(guiContext->tempBuffer, 64, "%d", *v);
			guiContext->addTiledXBox(p, width, guiContext->textures[ICN_INPUT], BOX_HEIGHT);
			p.y -= 1.0f;
			guiContext->addText(p, guiContext->tempBuffer);
		}		
	}

	// -------------------------------------------------------
	// input scalar
	// -------------------------------------------------------
	void InputScalar(int id, int index, float* v, float width = INPUT_BOX_WIDTH) {
		ZoneTracker z("IMGUI::InputScalar-F");
		int new_id = id + 1024 * index;
		v2 p = guiContext->position;
		p.x += (width + 10.0f) * index + guiContext->settings[GS_LABELSIZE];
		bool hot = isHot(new_id, p, v2(width, BOX_HEIGHT), width * 0.5f);
		bool selected = isBoxSelected(new_id, p, v2(width, BOX_HEIGHT));
		if ( selected) {
			sprintf_s(guiContext->inputText, 32, "%.2f", *v);
			guiContext->caretPos = strlen(guiContext->inputText);
			guiContext->active = new_id;
		}
		if (guiContext->active == new_id) {
			guiContext->addTiledXBox(p, width, guiContext->textures[ICN_INPUT_ACTIVE], BOX_HEIGHT);
			handleTextInput();
			*v = atof(guiContext->inputText);
			v2 cp = p;
			v2 cursorPos = ds::font::calculateLimitedSize(*guiContext->font, guiContext->inputText, guiContext->caretPos, CHAR_PADDING);
			cp.x = guiContext->position.x + TEXT_PADDING + (width + 10.0f) * index + cursorPos.x + guiContext->settings[GS_LABELSIZE];
			guiContext->addBox(cp, v2(2, BOX_HEIGHT - 4.0f), ds::Color(192, 0, 0, 255));
			p.y -= 1.0f;
			guiContext->addText(p, guiContext->inputText);
		}
		else {
			sprintf_s(guiContext->tempBuffer, 64, "%.2f", *v);
			guiContext->addTiledXBox(p, width, guiContext->textures[ICN_INPUT], BOX_HEIGHT);
			p.y -= 1.0f;
			guiContext->addText(p, guiContext->tempBuffer);
		}
	}

	// -------------------------------------------------------
	// input scalar
	// -------------------------------------------------------
	bool InputScalar(int id, int index, char* v,int maxLength,float width = INPUT_BOX_WIDTH) {
		ZoneTracker z("IMGUI::InputScalar-C");
		int new_id = id + 1024 * index;
		bool ret = false;
		v2 p = guiContext->position;
		p.x += (width + 10.0f) * index + guiContext->settings[GS_LABELSIZE];
		bool hot = isHot(new_id, p, v2(width, BOX_HEIGHT), width * 0.5f);
		bool selected = isBoxSelected(new_id, p, v2(width, BOX_HEIGHT));
		if (selected) {
			sprintf_s(guiContext->inputText, maxLength, "%s", v);
			guiContext->caretPos = strlen(guiContext->inputText);
			guiContext->active = new_id;
		}
		if (guiContext->active == new_id) {
			guiContext->addTiledXBox(p, width, guiContext->textures[ICN_INPUT_ACTIVE], BOX_HEIGHT);
			ret = handleTextInput();
			strncpy(v, guiContext->inputText, maxLength);
			v2 cp = p;
			v2 cursorPos = ds::font::calculateLimitedSize(*guiContext->font, guiContext->inputText, guiContext->caretPos, CHAR_PADDING);
			cp.x = guiContext->position.x + TEXT_PADDING + (width + 10.0f) * index + cursorPos.x + guiContext->settings[GS_LABELSIZE];
			guiContext->addBox(cp, v2(2, BOX_HEIGHT - 4.0f), ds::Color(192, 0, 0, 255));
			p.y -= 1.0f;
			guiContext->addText(p, guiContext->inputText);
			sprintf_s(v, maxLength, "%s", guiContext->inputText);
		}
		else {
			sprintf_s(guiContext->tempBuffer, 64, "%s", v);
			guiContext->addTiledXBox(p, width, guiContext->textures[ICN_INPUT], BOX_HEIGHT);
			p.y -= 1.0f;
			guiContext->addText(p, guiContext->tempBuffer);
		}
		return ret;
	}

	// -------------------------------------------------------
	// input string
	// -------------------------------------------------------
	bool Input(const char* label, char* str, int maxLength) {
		HashedId id = HashPointer(str);
		v2 p = guiContext->position;
		guiContext->addText(p, label);
		bool ret = InputScalar(id, 0, str, maxLength,300.0f);		
		guiContext->nextPosition();
		return ret;
	}

	// -------------------------------------------------------
	// input string without label
	// -------------------------------------------------------
	bool Input(char* str, int maxLength) {
		HashedId id = HashPointer(str);
		bool ret = InputScalar(id, 0, str, maxLength, 300.0f);
		v2 p = guiContext->position;
		guiContext->nextPosition();
		return ret;
	}

	// -------------------------------------------------------
	// input float
	// -------------------------------------------------------
	void InputFloat(const char* label, float* v) {
		HashedId id = HashPointer(v);
		v2 p = guiContext->position;		
		guiContext->addText(p, label);
		InputScalar(id, 0, v);		
		guiContext->nextPosition();
	}

	// -------------------------------------------------------
	// input int
	// -------------------------------------------------------
	void InputInt(const char* label, int* v) {		
		HashedId id = HashPointer(v);
		v2 p = guiContext->position;
		guiContext->addText(p, label);
		InputScalar(id, 0, v);		
		guiContext->nextPosition();
	}

	// -------------------------------------------------------
	// input int
	// -------------------------------------------------------
	void InputInt(const char* label, uint32_t* v) {
		HashedId id = HashPointer(v);
		int tmp = *v;
		InputScalar(id, 0, &tmp);
		v2 p = guiContext->position;
		guiContext->addText(p, label);
		guiContext->nextPosition();
		*v = tmp;
	}

	// -------------------------------------------------------
	// input int using steps
	// -------------------------------------------------------
	void InputFloat(const char* label, float* v, float minValue, float maxValue, float step) {
		HashedId id = HashPointer(v);
		v2 p = guiContext->position;
		guiContext->addText(p, label);
		p.x += guiContext->settings[GS_LABELSIZE];

		guiContext->addImage(p, guiContext->textures[ICN_MINUS], BOX_HEIGHT*0.5f);
		if (isBoxSelected(id, p, v2(BOX_HEIGHT, BOX_HEIGHT))) {
			*v -= step;			
		}
		float width = 100.0f;
		p.x += BOX_HEIGHT + 50.0f;
		guiContext->addImage(p, guiContext->textures[ICN_STEP_INPUT]);
		p.x -= 50.0f;
		if (isDragging(p, v2(100.0f, BOX_HEIGHT))) {
			int dx = (guiContext->cursorPosition.x - p.x) / 10;
			if (dx > 0) {
				*v = minValue + (maxValue - minValue) / step * ((float)dx / 10.0f);
			}
			else {
				*v = minValue;
			}
		}
		p.x += 100.0f;
		guiContext->addImage(p, guiContext->textures[ICN_PLUS], BOX_HEIGHT*0.5f);
		if (isBoxSelected(id, p, v2(BOX_HEIGHT, BOX_HEIGHT))) {
			*v += step;			
		}
		if (*v < minValue) {
			*v = minValue;
		}
		if (*v > maxValue) {
			*v = maxValue;
		}
		sprintf_s(guiContext->tempBuffer, 64, "%g", *v);
		v2 dim = getTextSize(guiContext->tempBuffer);
		p = guiContext->position;
		p.x += BOX_HEIGHT + (100.0f - dim.x) * 0.5f + guiContext->settings[GS_LABELSIZE];
		guiContext->addText(p, guiContext->tempBuffer);
		guiContext->nextPosition();
	}

	// -------------------------------------------------------
	// input int using steps
	// -------------------------------------------------------
	void InputInt(const char* label, int* v,int minValue,int maxValue,int step) {
		HashedId id = HashPointer(v);
		v2 p = guiContext->position;
		guiContext->addImage(p, guiContext->textures[ICN_MINUS], BOX_HEIGHT*0.5f);
		if (isBoxSelected(id, p, v2(BOX_HEIGHT, BOX_HEIGHT))) {
			*v -= step;			
		}
		float width = INPUT_BOX_WIDTH;
		p.x += BOX_HEIGHT;
		guiContext->addBox(p, v2(width, BOX_HEIGHT), guiContext->colors[CLR_INPUT]);		
		p.x += INPUT_BOX_WIDTH;
		guiContext->addImage(p, guiContext->textures[ICN_PLUS], BOX_HEIGHT*0.5f);
		if (isBoxSelected(id, p, v2(BOX_HEIGHT, BOX_HEIGHT))) {
			*v += step;			
		}
		if (*v < minValue) {
			*v = minValue;
		}
		if (*v > maxValue) {
			*v = maxValue;
		}
		sprintf_s(guiContext->tempBuffer, 64, "%d", *v);
		v2 dim = getTextSize(guiContext->tempBuffer);
		p = guiContext->position;
		p.x += BOX_HEIGHT + (INPUT_BOX_WIDTH - dim.x) * 0.5f;
		guiContext->addText(p, guiContext->tempBuffer);
		p = guiContext->position;
		p.x += INPUT_BOX_WIDTH + BOX_HEIGHT * 2.0f + 10.0f;
		guiContext->addText(p, label);
		guiContext->nextPosition();
	}

	// -------------------------------------------------------
	// input v2
	// -------------------------------------------------------
	void InputVec2(const char* label, v2* v) {
		HashedId id = HashPointer(v);
		v2 p = guiContext->position;
		guiContext->addText(p, label);
		InputScalar(id, 0, &v->x);
		InputScalar(id, 1, &v->y);	
		guiContext->nextPosition();
	}

	// -------------------------------------------------------
	// input color
	// -------------------------------------------------------
	void InputColor(const char* label, ds::Color* v) {
		int r = v->r * 255.0f;
		int g = v->g * 255.0f;
		int b = v->b * 255.0f;
		int a = v->a * 255.0f;
		v2 p = guiContext->position;
		guiContext->addText(p, label);
		HashedId hash = HashPointer(v);
		InputScalar(hash, 0, &r, 40.0f);
		InputScalar(hash, 1, &g, 40.0f);
		InputScalar(hash, 2, &b, 40.0f);
		InputScalar(hash, 3, &a, 40.0f);
		*v = ds::Color(r, g, b, a);
		p.x += guiContext->settings[GS_LABELSIZE] + 200.0f;
		guiContext->addBox(p, v2(BOX_HEIGHT, BOX_HEIGHT), *v);
		guiContext->nextPosition();
	}

	// -------------------------------------------------------
	// input vec3
	// -------------------------------------------------------
	void InputVec3(const char* label, v3* v) {
		HashedId id = HashPointer(v);
		v2 p = guiContext->position;
		guiContext->addText(p, label);
		InputScalar(id, 0, &v->x);
		InputScalar(id, 1, &v->y);
		InputScalar(id, 2, &v->z);		
		guiContext->nextPosition();
	}

	// -------------------------------------------------------
	// input rect
	// -------------------------------------------------------
	void InputRect(const char* label, ds::Rect* v) {
		HashedId id = HashPointer(v);
		v2 p = guiContext->position;
		guiContext->addText(p, label);
		int top = v->top;
		int left = v->left;
		int width = v->width();
		int height = v->height();
		InputScalar(id, 0, &top);
		InputScalar(id, 1, &left);
		InputScalar(id, 2, &width);
		InputScalar(id, 3, &height);
		*v = ds::Rect(top, left, width, height);		
		guiContext->nextPosition();
	}

	// -------------------------------------------------------
	// prepare combo box
	// -------------------------------------------------------
	void prepareComboBox(int id,int* offset,int size,int max) {		
		ZoneTracker z("IMGUI::prepareComboBox");
		float width = 200.0f;
		v2 p = guiContext->position;
		float height = max * BOX_HEIGHT;
		p.y += BOX_HEIGHT / 2.0f;
		guiContext->addTiledXYBox(p, v2(width, height), guiContext->textures[ICN_BOX_BACKGROUND], 10.0f);
		if (size > max)  {
			p = guiContext->position;
			p.x += width + BOX_HEIGHT * 0.5f;

			guiContext->addImage(p, guiContext->textures[ICN_ARROW_UP]);
			if (isBoxSelected(id, p, v2(BOX_HEIGHT, BOX_HEIGHT))) {
				*offset -= 1;
				if (*offset < 0) {
					*offset = 0;
				}
			}
			float sideHeight = height - 2.0f * BOX_HEIGHT;
			p.y -= sideHeight * 0.5f;
			p.y -= BOX_HEIGHT * 0.5f;
			p.x -= BOX_HEIGHT * 0.5f;
			guiContext->addBox(p, v2(BOX_HEIGHT, sideHeight), guiContext->colors[CLR_SLIDER]);

			p.x += BOX_HEIGHT * 0.5f;
			p.y = guiContext->position.y - (max - 1) * BOX_HEIGHT;
			guiContext->addImage(p, guiContext->textures[ICN_ARROW_DOWN]);
			// FIXME: seems to be half off
			if (isBoxSelected(id, p, v2(BOX_HEIGHT, BOX_HEIGHT))) {
				if ((*offset + max) < size) {
					*offset += 1;
				}
			}

			float f = static_cast<float>(*offset) / static_cast<float>(size - max);
			sideHeight = height - 3.0f * BOX_HEIGHT;
			float slid = sideHeight * f;
			p = guiContext->position;
			p.y -= BOX_HEIGHT;
			p.x += width + BOX_HEIGHT * 0.5f;
			p.y -= slid;
			guiContext->addImage(p, guiContext->textures[ICN_ARROW_LEFT]);
		}
	}

	// -------------------------------------------------------
	// combo box with model
	// -------------------------------------------------------	
	void ComboBox(AbstractComponentModel* model,int *offset,int max) {
		ZoneTracker z("IMGUI::ComboBox");
		HashedId id = HashPointer(model);
		prepareComboBox(id, offset, model->size(),max);
		float width = 200.0f;
		v2 p = guiContext->position;
		float height = max * BOX_HEIGHT;		
		bool hot = isHot(id, p, v2(width, height));
		int start = *offset;
		int end = *offset + max;
		if (end >= model->size()) {
			end = model->size();
		}
		for (int i = start; i < end; ++i) {
			if (isBoxSelected(id, p, v2(width, BOX_HEIGHT))) {
				model->select(i);
			}
			if (model->isSelected(i)) {
				guiContext->addBox(p, v2(width, BOX_HEIGHT), guiContext->colors[CLR_SELECTED_LINE]);
			}
			guiContext->addText(p, model->getLabel(i));
			p.y -= BOX_HEIGHT;
		}
		guiContext->nextPosition(height + BOX_HEIGHT);
	}

	// -------------------------------------------------------
	// combo box strings
	// -------------------------------------------------------	
	void ComboBox(const ds::Array<const char*>& entries, int* selected, int *offset, int max) {
		HashedId id = HashPointer(&entries);
		prepareComboBox(id, offset, entries.size(),max);
		float width = 200.0f;
		v2 p = guiContext->position;
		float height = max * BOX_HEIGHT;		
		p = guiContext->position;
		bool hot = isHot(id, p, v2(width, height));
		int start = *offset;
		int end = *offset + max;
		if (end >= entries.size()) {
			end = entries.size();
		}
		for (int i = start; i < end; ++i) {
			if (isBoxSelected(id, p, v2(width, BOX_HEIGHT))) {
				*selected = i;
			}
			if (*selected == i) {
				guiContext->addBox(p, v2(width, BOX_HEIGHT), guiContext->colors[CLR_SELECTED_LINE]);
			}				
			guiContext->addText(p, entries[i]);			
			p.y -= BOX_HEIGHT;
		}
		guiContext->nextPosition(height + BOX_HEIGHT);
	}

	// -------------------------------------------------------
	// prepare drop down box
	// -------------------------------------------------------	
	void prepareDropDownBox(HashedId id,int max,int* state,const char* selection) {
		float width = 200.0f;
		v2 p = guiContext->position;
		bool hot = isHot(id, p, v2(width, BOX_HEIGHT), width * 0.5f);
		if (isBoxSelected(id, p, v2(width, BOX_HEIGHT))) {
			if (*state == 0) {
				*state = 1;
			}
			else {
				guiContext->active = -1;
				*state = 0;
			}
		}
		guiContext->addBox(p, v2(width, BOX_HEIGHT), guiContext->colors[CLR_INPUT]);
		if (*state == 0) {
			guiContext->addImage(p, guiContext->textures[ICN_ARROW_RIGHT], 8.0f);
		}
		else {
			guiContext->addImage(p, guiContext->textures[ICN_ARROW_DOWN], 8.0f);
		}
		v2 tp = p;
		tp.x += 16.0f;
		guiContext->addText(tp, selection);

		if (*state == 1) {
			float height = max * BOX_HEIGHT;
			p.y -= BOX_HEIGHT / 2.0f;
			guiContext->addTiledXYBox(p, v2(width, height), guiContext->textures[ICN_BOX_BACKGROUND], 10.0f);
		}
	}

	// -------------------------------------------------------
	// drop down box
	// -------------------------------------------------------	
	void DropDownBox(AbstractComponentModel* model, int* state) {
		HashedId id = HashPointer("model");
		if (!model->hasSelection()) {
			prepareDropDownBox(id, model->size(), state, EMPTY_SELECTION);
		}
		else {
			prepareDropDownBox(id, model->size(), state, model->getLabel(model->getSelection()));
		}
		if (*state == 1) {
			float height = model->size() * BOX_HEIGHT;
			v2 p = guiContext->position;
			p.y -= BOX_HEIGHT;
			for (size_t i = 0; i < model->size(); ++i) {
				if (isBoxSelected(id, p, v2(200.0f, BOX_HEIGHT), false)) {
					model->select(i);
					*state = 0;
				}
				guiContext->addText(p, model->getLabel(i));
				p.y -= BOX_HEIGHT;
			}
			guiContext->position.y -= height;
			guiContext->position.y -= BOX_HEIGHT * 0.5f;
		}
		guiContext->nextPosition();
	}


	void DropDownBox(const char** entries, int num, int* selected, int* state) {
		HashedId id = HashPointer(&entries);
		if (*selected < 0 || *selected >= num) {
			prepareDropDownBox(id, num, state, EMPTY_SELECTION);
		}
		else {
			prepareDropDownBox(id, num, state, entries[*selected]);
		}
		if (*state == 1) {
			float height = num * BOX_HEIGHT;
			v2 p = guiContext->position;
			p.y -= BOX_HEIGHT;
			for (size_t i = 0; i < num; ++i) {
				if (isBoxSelected(id, p, v2(200.0f, BOX_HEIGHT), false)) {
					*selected = i;
					*state = 0;
				}
				guiContext->addText(p, entries[i]);
				p.y -= BOX_HEIGHT;
			}
			guiContext->position.y -= height;
			guiContext->position.y -= BOX_HEIGHT * 0.5f;
		}
		guiContext->nextPosition();
	}

	// -------------------------------------------------------
	// drop down box
	// -------------------------------------------------------	
	void DropDownBox(const ds::Array<const char*>& entries, int* selected, int* state) {
		HashedId id = HashPointer(&entries);
		int num = entries.size();
		if (*selected < 0 || *selected >= num) {
			prepareDropDownBox(id, num, state, EMPTY_SELECTION);
		}
		else {
			prepareDropDownBox(id, num, state, entries[*selected]);
		}
		if (*state == 1) {
			float height = num * BOX_HEIGHT;
			v2 p = guiContext->position;
			p.y -= BOX_HEIGHT;
			for (size_t i = 0; i < num; ++i) {
				if (isBoxSelected(id, p, v2(200.0f, BOX_HEIGHT), false)) {
					*selected = i;
					*state = 0;
				}
				guiContext->addText(p, entries[i]);
				p.y -= BOX_HEIGHT;
			}
			guiContext->position.y -= height;
			guiContext->position.y -= BOX_HEIGHT * 0.5f;
		}
		guiContext->nextPosition();
	}

	int ActionBar(const ds::Array<const char*>& entries) {
		HashedId id = HashPointer(&entries);
		int selected = -1;
		int num = entries.size();
		v2 p = guiContext->position;
		p.x += 10.0f;
		for (int i = 0; i < num; ++i) {
			v2 size = getTextSize(entries[i]);
			guiContext->addText(p, entries[i], size, -2.0f);
			v2 textSize = getTextSize(entries[i]) + v2(10, 6);
			p.y += 8.0f;
			if (isBoxSelected(id, p, v2(textSize.x, BOX_HEIGHT), false)) {
				selected = i;
			}
			p.x += getTextSize(entries[i]).x + 6.0f;
			guiContext->addBox(p, v2(2, 26.0f), guiContext->colors[CLR_INPUT]);
			p.x += 6.0f;
			p.y -= 8.0f;
		}
		guiContext->nextPosition(BOX_HEIGHT);
		return selected;
	}

	// -------------------------------------------------------
	// Progress bar
	// -------------------------------------------------------
	void ProgressBar(const char* label, float fraction) {
		HashedId id = HashPointer(label);
		// cut off 2px at each side for the border
		float sx = fraction - 4.0f;
		if (sx < 0.0f) {
			sx = 0.0f;
		}
		if (sx > 100.0f) {
			sx = 100.0f;
		}
		v2 p = guiContext->position;
		guiContext->addText(p, label);
		p.x += guiContext->settings[GS_LABELSIZE];
		float width = 100.0f;
		p.x += 50.0f;
		guiContext->addImage(p, guiContext->textures[ICN_STEP_INPUT]);
		p.x -= 48.0f;
		guiContext->addBox(p, v2(sx, 14.0f), guiContext->colors[CLR_PROGRESS]);
		sprintf_s(guiContext->tempBuffer, 64, "%g%%", fraction);
		v2 dim = getTextSize(guiContext->tempBuffer);
		p = guiContext->position;
		p.x += (100.0f - dim.x) * 0.5f + guiContext->settings[GS_LABELSIZE];
		guiContext->addText(p, guiContext->tempBuffer);
		guiContext->nextPosition();
	}

	// -------------------------------------------------------
	// Slider using steps
	// -------------------------------------------------------
	void Slider(const v2& pos,int* v, int minValue, int maxValue, int step = 1,const char* prefix = NULL) {
		float width = 255.0f;
		v2 p = pos;
		p.x += width * 0.5f;
		guiContext->addImage(p, guiContext->textures[ICN_COLOR_SLIDER]);
		p.x -= width * 0.5f;
		if (isDragging(p, v2(width, BOX_HEIGHT))) {
			int dx = (guiContext->cursorPosition.x - p.x);
			if (dx > 0) {
				*v = minValue + (maxValue - minValue) / 255 * dx;
			}
			else {
				*v = minValue;
			}
		}
		if (*v < minValue) {
			*v = minValue;
		}
		if (*v > maxValue) {
			*v = maxValue;
		}
		if (prefix != NULL) {
			sprintf_s(guiContext->tempBuffer, 64, "%s %d", prefix, *v);
		}
		else {
			sprintf_s(guiContext->tempBuffer, 64, "%d", *v);
		}
		v2 dim = getTextSize(guiContext->tempBuffer);
		p = pos;
		p.x += (width - dim.x) * 0.5f;
		guiContext->addText(p, guiContext->tempBuffer);
	}

	// -------------------------------------------------------
	// input color
	// -------------------------------------------------------
	void ColorSlider(const char* label, ds::Color* v,int* state) {
		static char* PREFIXES[] = { "r:", "g:", "b:", "a:" };
		int colors[4];
		colors[0] = v->r * 255.0f;
		colors[1] = v->g * 255.0f;
		colors[2] = v->b * 255.0f;
		colors[3] = v->a * 255.0f;
		v2 p = guiContext->position;
		guiContext->addText(p, label);
		p.x += guiContext->settings[GS_LABELSIZE];
		guiContext->addBox(p, v2(BOX_HEIGHT, BOX_HEIGHT), *v);
		p.x += BOX_HEIGHT * 1.5f;
		if (isClicked(p, v2(BOX_HEIGHT, BOX_HEIGHT))) {
			if (*state == 0) {
				*state = 1;
			}
			else {
				*state = 0;
			}
		}
		if (*state == 0) {
			guiContext->addImage(p, guiContext->textures[ICN_ARROW_LEFT]);
			guiContext->nextPosition();
		}
		else {
			guiContext->addImage(p, guiContext->textures[ICN_ARROW_DOWN]);
			p.y -= 20.0f;
			p.x = guiContext->position.x;
			for (int i = 0; i < 4; ++i) {
				Slider(p, &colors[i], 0, 255, 1, PREFIXES[i]);
				p.y -= 20.0f;
			}
			p.x += 80.0f;
			guiContext->nextPosition(80.0f + guiContext->settings[GS_LINE_HEIGHT]);
		}
		*v = ds::Color(colors);
		
	}


	void Slider(const char* label, float* v, float minValue, float maxValue, float step) {
		HashedId id = HashPointer(v);
		v2 p = guiContext->position;
		guiContext->addText(p, label);
		p.x += guiContext->settings[GS_LABELSIZE];
		float width = 100.0f;
		p.x += 50.0f;
		guiContext->addImage(p, guiContext->textures[ICN_STEP_INPUT]);
		p.x -= 55.0f;
		if (isDragging(p, v2(100.0f, BOX_HEIGHT))) {
			int dx = (guiContext->cursorPosition.x - p.x) / 10;
			if (dx > 0) {
				*v = minValue + (maxValue - minValue) / step * ((float)dx / 10.0f);
			}
			else {
				*v = minValue;
			}
		}
		sprintf_s(guiContext->tempBuffer, 64, "%g", *v);
		v2 dim = getTextSize(guiContext->tempBuffer);
		p = guiContext->position;
		p.x += (100.0f - dim.x) * 0.5f + guiContext->settings[GS_LABELSIZE];
		guiContext->addText(p, guiContext->tempBuffer);
		guiContext->nextPosition();
	}

	// -------------------------------------------------------
	// Separator
	// -------------------------------------------------------	
	void Separator() {
		v2 p = guiContext->position;
		p.y += 8.0f;
		guiContext->addBox(p, v2(100, 2), ds::Color(32, 32, 32, 255), true);
		guiContext->position.y -= 6.0f;
	}

	// -------------------------------------------------------
	// checkbox
	// -------------------------------------------------------	
	void CheckBox(const char* label, bool* selected) {
		HashedId id = HashId(label);
		v2 p = guiContext->position;
		guiContext->addText(p, label);
		p.x += guiContext->settings[GS_LABELSIZE];
		if (*selected) {
			guiContext->addImage(p, guiContext->textures[ICN_CHECKBOX_SELECTED], BOX_HEIGHT * 0.5f);			
		}
		else {
			guiContext->addImage(p, guiContext->textures[ICN_CHECKBOX], BOX_HEIGHT * 0.5f);
		}
		if (isBoxSelected(id, p, v2(BOX_HEIGHT, BOX_HEIGHT))) {
			*selected = !*selected;
		}
		v2 textDim = getTextSize(label);
		textDim.x += BOX_HEIGHT;
		bool hot = isHot(id, p, textDim,textDim.x * 0.5f);
		guiContext->nextPosition();
	}
	
	// -------------------------------------------------------
	// Image
	// -------------------------------------------------------	
	void Image(const char* label, const ds::Rect& r, int textureID, const ds::Color& clr) {
		v2 p = guiContext->position;
		guiContext->addText(p, label);
		v2 size = v2(r.width(), r.height());
		size += v2(4, 4);
		p.x += guiContext->settings[GS_LABELSIZE];		
		p.y -= r.height() * 0.5f;
		p += v2(-2, 0);
		guiContext->addBox(p, size, guiContext->colors[CLR_IMAGE_BACKGROUND]);		
		p.x += r.width() * 0.5f;
		p += v2(2, 0);
		guiContext->addExternalImage(p, math::buildTexture(r), textureID, clr);
		guiContext->nextPosition(size.y + guiContext->settings[GS_LINE_HEIGHT]);
	}

	// -------------------------------------------------------
	// button
	// -------------------------------------------------------	
	bool Button(const char* label) {
		v2 textDim = getTextSize(label);
		float width = textDim.x + BUTTON_PADDING * 2.0f;
		v2 p = guiContext->position;
		HashedId id = HashId(label);
		bool hot = isHot(id, p, v2(width, BUTTON_HEIGHT),width * 0.5f);
		guiContext->addTiledXBox(p, width, guiContext->textures[ICN_BUTTON], BUTTON_HEIGHT);
		p.x = guiContext->position.x + (width - textDim.x) / 2.0f;
		guiContext->addText(p,label, textDim);
		guiContext->nextPosition(guiContext->settings[GS_LINE_HEIGHT] + 4.0f);
		return isBoxSelected(id, p, v2(width, BUTTON_HEIGHT));
	}

	int ButtonBar(const ds::Array<const char*>& entries) {
		int ret = -1;
		beginGroup();
		for (int i = 0; i < entries.size(); ++i) {
			if (Button(entries[i])) {
				ret = i;
			}
		}
		endGroup();
		return ret;
	}

	// -------------------------------------------------------
	// Histogram
	// -------------------------------------------------------	
	void Histogram(float* values, int num, float minValue, float maxValue, float step) {
		v2 p = guiContext->position;
		HashedId id = HashPointer(values);
		float width = 200.0f;
		float height = 100.0f;
		float barWidth = 10.0f;
		p.y -= height / 2.0f;
		float delta = maxValue - minValue;
		if (delta == 0.0f) {
			delta = 1.0f;
		}
		float st = width / static_cast<float>(num - 1);
		guiContext->addBox(p, v2(width + barWidth, height), guiContext->colors[CLR_INPUT]);
		p.x += width + BOX_HEIGHT;
		p.y += height / 2.0f;
		char buffer[16];
		sprintf_s(buffer, 16, "%.2f", maxValue);
		guiContext->addText(p, buffer);
		p.y -= height;
		sprintf_s(buffer, 16, "%.2f", minValue);
		guiContext->addText(p, buffer);
		for (int i = 0; i < num; ++i) {
			float v = values[i];
			if (v > maxValue) {
				v = maxValue;
			}
			float current = v / delta;
			float yp = current * height;
			p = guiContext->position;
			p.y -= ( height - yp * 0.5f);
			p.x += static_cast<float>(i) * st;
			guiContext->addBox(p, v2(barWidth, yp), guiContext->colors[CLR_SELECTED_LINE]);
		}
		step = delta / 10.0f;
		int d = delta / step + 1;
		for (int i = 0; i < d; ++i) {
			p = guiContext->position;
			float current = 1.0f - (step*i) / delta;
			float yp = current * height;
			p.y -= yp;
			guiContext->addBox(p, v2(width + barWidth, 1.0f), guiContext->colors[CLR_INPUT_EDIT]);
		}
		guiContext->nextPosition();
	}

	// -------------------------------------------------------
	// Diagram
	// -------------------------------------------------------	
	void DiagramInternal(const v2& pos, float* values, int num, float minValue, float maxValue, float step) {
		v2 p = pos;
		HashedId id = HashPointer(values);
		float width = 200.0f;
		float height = 100.0f;
		p.y -= height / 2.0f;
		float delta = (maxValue - minValue);
		if (delta == 0.0f) {
			delta = 1.0f;
		}
		float st = width / static_cast<float>(num - 1);
		guiContext->addBox(p, v2(width, height), guiContext->colors[CLR_INPUT]);
		p.x += width + BOX_HEIGHT;
		p.y += height / 2.0f;
		char buffer[16];
		sprintf_s(buffer, 16, "%g", maxValue);
		guiContext->addText(p, buffer);
		p.y -= height;
		sprintf_s(buffer, 16, "%g", minValue);
		guiContext->addText(p, buffer);

		for (int i = 0; i < num; ++i) {
			float v = values[i];
			if (v > maxValue) {
				v = maxValue;
			}
			if (v < minValue) {
				v = minValue;
			}
			float norm = (v - minValue) / delta;
			float yp = norm * height;
			p = pos;
			p.y -= yp;
			p.x += static_cast<float>(i)* st - 2.0f;
			guiContext->addBox(p, v2(4, 4), guiContext->colors[CLR_SELECTED_LINE]);
		}

		step = delta / 10.0f;
		int d = delta / step + 1;
		for (int i = 0; i < d; ++i) {
			p = pos;
			float current = 1.0f - (step*i) / delta;
			float yp = current * height;
			p.y -= yp;
			guiContext->addBox(p, v2(width, 1.0f), guiContext->colors[CLR_INPUT_EDIT]);
		}
		guiContext->nextPosition(height + guiContext->settings[GS_LINE_HEIGHT]);
	}

	void Diagram(float* values, int num, float minValue, float maxValue, float step) {
		DiagramInternal(guiContext->position, values, num, minValue, maxValue, step);
	}

	void Diagram(const char* label, float* values, int num, float minValue, float maxValue, float step) {
		v2 p = guiContext->position;
		guiContext->addText(p, label);
		p.x += guiContext->settings[GS_LABELSIZE];
		DiagramInternal(p, values, num, minValue, maxValue, step);
	}



	// -------------------------------------------------------
	// push settings
	// -------------------------------------------------------	
	void PushSetting(GUISetting setting, float value) {
		guiContext->backup[setting] = guiContext->settings[setting];
		guiContext->settings[setting] = value;
	}

	// -------------------------------------------------------
	// pop settings
	// -------------------------------------------------------	
	void PopSetting(GUISetting setting) {
		guiContext->settings[setting] = guiContext->backup[setting];
	}

	// -------------------------------------------------------
	// intialize gui
	// -------------------------------------------------------	
	void initialize(bool editorMode) {
		assert(guiContext == 0);
		guiContext = new GUIContext;
		guiContext->textureID = ds::renderer::loadTexture("gui");
		assert(guiContext->textureID != -1);
		guiContext->font = ds::renderer::createBitmapFont("gui_font", guiContext->textureID);
		ds::FontDefinition fdf;
		fdf.startChar = 32;
		fdf.endChar = 128;
		fdf.width = 209;
		fdf.height = 92;
		fdf.padding = 1;
		fdf.textureSize = 1024.0f;
		fdf.charHeight = 14;
		fdf.startX = 0;
		fdf.startY = 0;
		fdf.gridHeight = 14;

		guiContext->font->intialize(fdf);
		ds::renderer::initializeBitmapFont(guiContext->font, guiContext->textureID);
		guiContext->clicked = false;
		guiContext->released = false;
		guiContext->buttonPressed = false;
		guiContext->active = -1;
		guiContext->hot = -1;
		guiContext->dragging = false;
		// some pre build icons / textures
		guiContext->textures[ICN_ARROW_RIGHT]       = math::buildTexture(  0.0f, 256.0f,  16.0f, 16.0f);
		guiContext->textures[ICN_ARROW_DOWN]        = math::buildTexture(  0.0f, 272.0f,  16.0f, 16.0f);
		guiContext->textures[ICN_ARROW_LEFT]        = math::buildTexture(  0.0f, 288.0f,  16.0f, 16.0f);
		guiContext->textures[ICN_ARROW_UP]          = math::buildTexture(  0.0f, 304.0f,  16.0f, 16.0f);
		guiContext->textures[ICN_PLUS]              = math::buildTexture( 16.0f, 238.0f,  18.0f, 18.0f);
		guiContext->textures[ICN_MINUS]             = math::buildTexture( 16.0f, 220.0f,  18.0f, 18.0f);
		guiContext->textures[ICN_CHECKBOX]          = math::buildTexture(  0.0f, 352.0f,  16.0f, 16.0f);
		guiContext->textures[ICN_CHECKBOX_SELECTED] = math::buildTexture(  0.0f, 368.0f,  16.0f, 16.0f);
		guiContext->textures[ICN_DRAG_BOX]          = math::buildTexture(  0.0f, 384.0f,  16.0f, 16.0f);
		guiContext->textures[ICN_BUTTON]            = math::buildTexture(105.0f, 155.0f, 150.0f, 24.0f);
		guiContext->textures[ICN_INPUT]             = math::buildTexture(160.0f,   0.0f, 150.0f, BOX_HEIGHT);
		guiContext->textures[ICN_STEP_INPUT]        = math::buildTexture( 40.0f, 220.0f, 100.0f, BOX_HEIGHT);
		guiContext->textures[ICN_INPUT_ACTIVE]      = math::buildTexture(160.0f, 160.0f, 150.0f, BOX_HEIGHT);
		guiContext->textures[ICN_HEADER_BOX]        = math::buildTexture(140.0f,   0.0f, 150.0f, 18.0f);
		guiContext->textures[ICN_PANEL_BACKGROUND]  = math::buildTexture( 30.0f, 370.0f, 100.0f, 100.0f);
		guiContext->textures[ICN_BOX_BACKGROUND]    = math::buildTexture( 30.0f, 500.0f, 100.0f, 100.0f);
		guiContext->textures[ICN_SEPARATOR]         = math::buildTexture(140.0f,   0.0f, 150.0f, 2.0f);
		guiContext->textures[ICN_STEP_INPUT_SMALL]  = math::buildTexture( 60.0f, 220.0f,  50.0f, BOX_HEIGHT);
		guiContext->textures[ICN_COLOR_SLIDER]      = math::buildTexture(140.0f, 160.0f, 255.0f, BOX_HEIGHT);
		
		guiContext->colors[CLR_PANEL_BACKGROUND] = ds::Color( 32,  32,  32, 255);
		guiContext->colors[CLR_PANEL_HEADER]     = ds::Color(  0, 111, 204, 255);
		guiContext->colors[CLR_INPUT]            = ds::Color( 41,  46,  52, 255);
		guiContext->colors[CLR_INPUT_EDIT]       = ds::Color(128, 128, 128, 255);
		guiContext->colors[CLR_BUTTON]           = ds::Color( 89, 101, 117, 255);
		guiContext->colors[CLR_BUTTON_HOVER]     = ds::Color( 66,  76,  88, 255);
		guiContext->colors[CLR_SELECTED_LINE]    = ds::Color(128,   0, 128, 255);
		guiContext->colors[CLR_SLIDER]           = ds::Color( 48,  48,  48, 255);
		guiContext->colors[CLR_PROGRESS]         = ds::Color(140,   7,   7, 255);
		guiContext->colors[CLR_IMAGE_BACKGROUND] = ds::Color(  8,   8,   8, 255);

		guiContext->settings[GS_LINE_HEIGHT] = 24.0f;
		guiContext->settings[GS_ALPHA] = 1.0f;
		guiContext->settings[GS_LABELSIZE] = 80.0f;
		guiContext->editorMode = editorMode;
		guiContext->ready = true;
	}

	// -------------------------------------------------------
	// shutdown
	// -------------------------------------------------------	
	void shutdown() {
		if (guiContext != 0)  {
			delete guiContext;
		}
	}

	// -------------------------------------------------------
	// debug window
	// -------------------------------------------------------	
	void debugWindow() {
		GUIWindow& win = guiContext->window;
		if (win.calls.size() > 0) {
			for (int i = 0; i < win.calls.size(); ++i) {
				const DrawCall& call = win.calls[i];
				LOG << "type: " << call.type << " position: " << DBG_V2(call.position) << " size: " << DBG_V2(call.size);
			}

		}
	}

	// -------------------------------------------------------
	// get panel dimension
	// -------------------------------------------------------	
	v2 getPanelDimension() {
		v2 start = guiContext->startPosition;
		v2 dim = v2(0, 2000);
		GUIWindow& win = guiContext->window;
		for (int i = 0; i < win.calls.size(); ++i) {
			const DrawCall& call = win.calls[i];
			float endX = call.position.x + call.size.x;
			float endY = call.position.y - call.size.y * 0.5f;
			if (endX > dim.x) {
				dim.x = endX;
			}
			if (endY < dim.y) {
				dim.y = endY;
			}
		}
		if (guiContext->useHeader) {
			v2 hd = getTextSize(guiContext->header);
			hd.x += guiContext->startPosition.x;
			if (hd.x > dim.x) {
				dim.x = hd.x;
			}
		}
		dim.x = dim.x - start.x;
		dim.y = start.y - dim.y - BOX_HEIGHT;		
		if (guiContext->editorMode) {
			dim.x = 500.0f;
		}
		return dim;
	}
	// -------------------------------------------------------
	// end panel
	// -------------------------------------------------------	
	void end() {
		ZoneTracker z("IMGUI::end");
		assert(guiContext->ready);
		guiContext->spriteBuffer->begin();
		//int current = ds::sprites::getCurrentTextureID();
		//ds::sprites::setTexture(guiContext->textureID);
		// get dimension of entire panel
		v2 dim = getPanelDimension();
		// FIXME: check if header text extends dim		
		dim += v2(20, 20);
		GUIWindow& win = guiContext->window;
		// draw header box
		v2 p = guiContext->startPosition;
		ds::Color mainColor = ds::Color::WHITE;
		mainColor.a = guiContext->settings[GS_ALPHA];
		v2 startPos = guiContext->startPosition;
		if (guiContext->useHeader) {			
			p.x -= 10.0f;
			guiContext->spriteBuffer->drawTiledX(p, dim.x, guiContext->textures[ICN_HEADER_BOX], 18.0f, mainColor);
			// draw text
			p.y -= 8.0f;
			p.x = startPos.x + 10.0f;
			guiContext->spriteBuffer->drawText(guiContext->font, p.x, p.y, guiContext->header, 2);
			/*
			// draw icon
			p.x = startPos.x;// +.0f;
			p.y = startPos.y;
			if (guiContext->visible) {
				ds::sprites::draw(p, guiContext->textures[ICN_ARROW_DOWN]);
			}
			else {
				ds::sprites::draw(p, guiContext->textures[ICN_ARROW_RIGHT]);
			}
			*/
		}
		// draw panel background
		if (guiContext->visible) {
					
			v2 p = startPos;
			if (guiContext->modal) {
				p = v2(512, 384);
			}			
			p.x = startPos.x - 10.0f;
			if (guiContext->useHeader) {
				p.y = startPos.y - BOX_HEIGHT * 0.5f;
			}
			guiContext->spriteBuffer->drawTiledXY(p, dim, guiContext->textures[ICN_PANEL_BACKGROUND], 10.0f, mainColor);


			if (win.calls.size() > 0) {
				for (int i = 0; i < win.calls.size(); ++i) {
					const DrawCall& call = win.calls[i];
					ds::Color clr = call.color;
					clr.a = guiContext->settings[GS_ALPHA];
					if (call.type == DCT_BOX) {
						if (call.tilingDef == TD_NONE) {
							guiContext->spriteBuffer->draw(call.position, buildBoxTexture(call.size.x, call.size.y), 0.0f, v2(1.0f, 1.0f), clr);
						}
						else {
							v2 cp = call.position;
							float sx = dim.x - 20.0f;
							cp.x += dim.x * 0.5f - 60.0f;
							guiContext->spriteBuffer->draw(cp, buildBoxTexture(sx, call.size.y), 0.0f, v2(1.0f, 1.0f), clr);
						}
					}
					else if (call.type == DCT_TEXT) {
						clr.a = 1.0f;
						const char* text = win.textBuffer.data + call.textIndex;
						guiContext->spriteBuffer->drawText(guiContext->font, call.position.x, call.position.y, text, call.padding);
					}
					else if (call.type == DCT_IMAGE) {
						if (call.tilingDef == TD_NONE) {
							guiContext->spriteBuffer->draw(call.position, call.texture, 0.0f, call.size, clr);
						}
						else  if (call.tilingDef == TD_TILE_X) {
							guiContext->spriteBuffer->drawTiledX(call.position, call.size.x, call.texture, call.additional, clr);
						}
						else  if (call.tilingDef == TD_TILE_BOTH) {
							guiContext->spriteBuffer->drawTiledXY(call.position, call.size, call.texture, call.additional, clr);
						}
					}
					else if (call.type == DCT_HEADER) {
						v2 p = call.position;
						p.x -= 10.0f;
						guiContext->spriteBuffer->drawTiledX(p, dim.x, guiContext->textures[ICN_HEADER_BOX], 16.0f, clr);
						// draw text
						p.y -= 8.0f;
						p.x += 20.0f;
						const char* text = win.textBuffer.data + call.textIndex;
						guiContext->spriteBuffer->drawText(guiContext->font, p.x, p.y, text, call.padding);
					}
					else if (call.type == DCT_EXTERNAL_IMAGE) {
						//int current = ds::sprites::getCurrentTextureID();
						//ds::sprites::setTexture((int)call.additional);
						//ds::sprites::draw(call.position, call.texture, 0.0f, 1.0f, 1.0f, clr);
						//ds::sprites::setTexture(current);
					}
				}
				
			}
		}
		//ds::sprites::setTexture(current);
		guiContext->spriteBuffer->end();
		guiContext->position.y -= 8.0f;
	}

	// -------------------------------------------
	// pre defined dialogs
	// -------------------------------------------
	
	InputDialog::InputDialog() : _active(false) {
		_text[0] = '\0';
	}

	int InputDialog::showEmbedded(const char* header, const char* label) {
		if (!_active) {
			_text[0] = '\0';
			_active = true;
			_button = 0;
		}
		else {
			int state = 1;
			gui::Header("Input");
			gui::Label(header);
			gui::Input(label, _text, 32);
			gui::beginGroup();
			if (gui::Button("OK")) {
				_button = 1;
				_active = false;
			}
			if (gui::Button("Cancel")) {
				_button = 2;
				_active = false;
			}
			gui::endGroup();
		}
		return _button;
	}

	int InputDialog::show(const char* header, const char* label) {
		if (!_active) {
			_text[0] = '\0';
			_active = true;
			_button = 0;
		}
		else {
			int state = 1;
			gui::begin("Input",&state);
			gui::Label(header);
			gui::Input(label, _text, 32);
			gui::beginGroup();
			if (gui::Button("OK")) {
				_button = 1;
				_active = false;
			}
			if (gui::Button("Cancel")) {
				_button = 2;
				_active = false;
			}
			gui::endGroup();
			gui::end();
		}
		return _button;
	}
	const char* InputDialog::getText() const {
		return _text;
	}

}
