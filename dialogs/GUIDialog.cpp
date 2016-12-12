#include "GUIDialog.h"
#include "core\log\Log.h"
#include "core\string\StringUtils.h"
#include "..\utils\font.h"
#include "core\base\Assert.h"
#include "..\resources\ResourceContainer.h"
#include "..\base\InputSystem.h"

namespace ds {

	GUIDialog::GUIDialog(const ModalDialogSettings& modalSettings) : JSONAssetFile("fileName") {
		_font = modalSettings.font;
		_hoverCallback = 0;
		_bitmapFont = res::getFont(_font);
		_sprites = graphics::getSpriteBuffer();
		_idIndex = 0;
		_active = false;
		for (int i = 0; i < MAX_GUID; ++i) {
			_ids[i].id = -1;
			_ids[i].index = -1;
		}
	}

	GUIDialog::GUIDialog(const GUIDialogDescriptor& descriptor,const char* fileName) : JSONAssetFile(fileName) {
		_font = descriptor.font;
		_hoverCallback = 0;
		_bitmapFont = res::getFont(_font);
		_sprites = graphics::getSpriteBuffer();
		_idIndex = 0;
		_active = false;
		for (int i = 0; i < MAX_GUID; ++i) {
			_ids[i].id = -1;
			_ids[i].index = -1;
		}
	}

	// -------------------------------------------------------
	// clear
	// -------------------------------------------------------
	void GUIDialog::clear() {
		_timers.clear();
		_idIndex = 0;
		_items.clear();
		_vertices.clear();
		for (int i = 0; i < MAX_GUID; ++i) {
			_ids[i].id = -1;
			_ids[i].index = -1;
		}
	}

	// -------------------------------------------------------
	// get index by id
	// -------------------------------------------------------
	int GUIDialog::getIndexByID(int id) {
		for (int i = 0; i < MAX_GUID; ++i) {
			if (_ids[i].id == id) {
				return i;
			}
		}
		FAIL("Not a valid id - no matching index found - id: %d", id);
		return -1;
	}
	
	// -------------------------------------------------------
	// Destructor
	// -------------------------------------------------------
	GUIDialog::~GUIDialog(void) {		
	}

	// -------------------------------------------------------
	// add number
	// -------------------------------------------------------
	GUID GUIDialog::addNumber(int id, const v2& position, int value, int length, const v2& scale, const Color& color, bool centered) {
		GUID& gid = _ids[_idIndex++];
		XASSERT(gid.id == -1, "The id %d is already in use", id);
		gid.id = id;
		gid.index = _items.size();
		v2 p = position;
		if (centered) {
			p.x = graphics::getScreenWidth() * 0.5f;
		}
		DialogItem item;
		item.pos = p;
		item.centered = centered;
		item.color = color;
		item.rotation = 0.0f;
		item.scale = scale;
		item.type = GIT_NUMBERS;
		item.index = _vertices.size();
		item.id = id;
		item.tmp = length;
		item.visible = true;
		char text[16];
		string::formatInt(value, text, length);
		v2 size = font::calculateSize(_font, text, 2);
		int sx = -size.x * 0.5f;
		int sy = -size.y * 0.5f;
		item.num = addTextVertices(text, sx, sy);		
		_items.push_back(item);

		return gid;
	}

	void GUIDialog::setNumber(int id, int value) {
		int idx = getIndexByID(id);
		XASSERT(idx != -1, "No matching GUI item for id: %d", id);
		XASSERT(idx < MAX_GUID, "Not a valid index - id: %d", id);
		const GUID& gid = _ids[idx];		
		const DialogItem& item = _items[gid.index];
		char text[16];
		string::formatInt(value, text, item.tmp);
		int padding = 2;
		int len = strlen(text);
		v2 size = font::calculateSize(_font, text, 2);
		int sx = -size.x * 0.5f;
		int sy = -size.y * 0.5f;
		for (int cnt = 0; cnt < len; ++cnt) {
			char c = text[cnt];
			const ds::Texture& t = _bitmapFont->get(c);
			float dimX = t.dim.x * item.scale.x;
			float dimY = t.dim.y * item.scale.y;
			_vertices[item.index + cnt].texture = t;
			_vertices[item.index + cnt].offset = v2(sx + dimX * 0.5f, dimY * 0.5f);
			sx += dimX + padding;
		}

	}

	// -------------------------------------------------------
	// Add static image
	// -------------------------------------------------------
	GUID GUIDialog::addImage(int id, int x, int y, const Rect& textureRect, const v2& scale, bool centered, const Color& color) {
		v2 p = v2(x,y);
		if (centered) {
			p.x = graphics::getScreenWidth() * 0.5f;
		}
		DialogItem item;
		item.pos = p;
		item.centered = centered;
		item.color = color;
		item.rotation = 0.0f;
		item.scale = scale;
		item.type = GIT_IMAGE;
		item.id = id;
		item.visible = true;
		item.index = _vertices.size();
		DialogVertex v;
		v.offset = v2(0, 0);
		v.texture = math::buildTexture(textureRect);
		_vertices.push_back(v);
		item.num = 1;
		_items.push_back(item);


		GUID& gid = _ids[_idIndex++];
		XASSERT(gid.id == -1, "The id %d is already in use", id);
		gid.id = id;
		gid.index = _items.size() - 1;
		return gid;
	}

	// -------------------------------------------------------
	// update image
	// -------------------------------------------------------
	void GUIDialog::updateImage(int id, int x, int y, const Rect& textureRect, bool centered) {
		int idx = getIndexByID(id);
		XASSERT(idx < MAX_GUID, "Not a valid index - id: %d", id);
		const GUID& gid = _ids[idx];
		XASSERT(gid.id != -1, "No matching GUI item for %d", id);
		const DialogItem& item = _items[gid.index];
		XASSERT(item.type == GIT_IMAGE, "This is not an image");
		_vertices[item.index].texture = math::buildTexture(textureRect);
		// FIXME: update position
	}

	// -------------------------------------------------------
	// Add image link
	// -------------------------------------------------------
	GUID GUIDialog::addImageButton(int id,int x,int y,const Rect& textureRect,bool centered) {
		GUID& gid = _ids[_idIndex++];
		XASSERT(gid.id == -1, "The id %d is already in use", id);
		gid.id = id;
		gid.index = _items.size();
		v2 p = v2(x, y);
		if (centered) {
			p.x = graphics::getScreenWidth() * 0.5f;
		}
		DialogItem item;
		item.pos = p;
		item.centered = centered;
		item.color = Color::WHITE;
		item.rotation = 0.0f;
		item.scale = v2(1, 1);
		item.type = GIT_IMAGE_BUTTON;
		item.id = id;
		item.visible = true;
		item.index = _vertices.size();
		float w = textureRect.width();
		float h = textureRect.height();
		item.boundingRect = Rect(h * 0.5f, w * -0.5f, w, -h);
		DialogVertex v;
		v.offset = v2(0, 0);
		v.texture = math::buildTexture(textureRect);
		_vertices.push_back(v);
		item.num = 1;
		_items.push_back(item);
		return gid;
	}

	// -------------------------------------------------------
	// add timer
	// -------------------------------------------------------
	GUID GUIDialog::addTimer(int id,int x, int y, const v2& scale, const Color& color, bool centered) {
		GUID& gid = _ids[_idIndex++];
		XASSERT(gid.id == -1, "The id %d is already in use", id);
		gid.id = id;
		gid.index = _items.size();
		v2 p = v2(x, y);
		if (centered) {
			p.x = graphics::getScreenWidth() * 0.5f;
		}
		DialogItem item;
		item.pos = p;
		item.centered = centered;
		item.color = color;
		item.rotation = 0.0f;
		item.scale = scale;
		item.id = id;
		item.type = GIT_TIMER;
		item.visible = true;
		item.index = _vertices.size();
		const char* text = "00:00";		
		item.num = addTextVertices(text, 0, 0);
		GameTimer timer;		
		item.tmp = _timers.size();
		_timers.push_back(timer);
		_items.push_back(item);
		return gid;
	}

	// -------------------------------------------------------
	// Adds a text
	// -------------------------------------------------------
	GUID GUIDialog::addText(int id,int x,int y,const char* text,const Color& color, const v2& scale,bool centered) {
		GUID& gid = _ids[_idIndex++];
		XASSERT(gid.id == -1, "The id %d is already in use", id);
		gid.id = id;
		gid.index = _items.size();
		v2 p = v2(x, y);
		if (centered) {
			p.x = graphics::getScreenWidth() * 0.5f;
		}
		DialogItem item;
		item.pos = p;
		item.centered = centered;
		item.color = color;
		item.rotation = 0.0f;
		item.scale = scale;
		item.type = GIT_TEXT;
		item.id = id;
		item.visible = true;
		item.index = _vertices.size();

		v2 size = font::calculateSize(_font, text, 2);
		int sx = -size.x * 0.5f;
		int sy = -size.y * 0.5f;
		int len = addTextVertices(text, sx, sy);
		item.tmp = len;
		item.num = len;
		_items.push_back(item);

		return gid;
	}

	// -------------------------------------------------------
	// reset specific timer
	// -------------------------------------------------------
	void GUIDialog::resetTimer(int id) {
		int idx = getIndexByID(id);
		XASSERT(idx >= 0 && idx < MAX_GUID, "Not a valid index - id: %d", id);
		const GUID& gid = _ids[idx];
		const DialogItem& item = _items[gid.index];
		XASSERT(item.type == GIT_TIMER,"The GUI item %d is not a timer",id);
		_timers[item.tmp].reset();
	}

	// -------------------------------------------------------
	// start specific timer
	// -------------------------------------------------------
	void GUIDialog::startTimer(int id) {
		int idx = getIndexByID(id);
		XASSERT(idx >= 0 && idx < MAX_GUID, "Not a valid index - id: %d", id);
		const GUID& gid = _ids[idx];
		const DialogItem& item = _items[gid.index];
		XASSERT(item.type == GIT_TIMER, "The GUI item %d is not a timer", id);
		_timers[item.tmp].start();
	}

	// -------------------------------------------------------
	// get specific timer
	// -------------------------------------------------------
	GameTimer* GUIDialog::getTimer(int id) {
		int idx = getIndexByID(id);
		XASSERT(idx >= 0 && idx < MAX_GUID, "Not a valid index - id: %d", id);
		const GUID& gid = _ids[idx];
		const DialogItem& item = _items[gid.index];
		XASSERT(item.type == GIT_TIMER, "The GUI item %d is not a timer", id);
		return &_timers[item.tmp];
	}

	// -------------------------------------------------------
	// Update text
	// -------------------------------------------------------
	void GUIDialog::updateText(int id,int x,int y,const char* text,const Color& color, const v2& scale,bool centered) {
		int idx = getIndexByID(id);
		XASSERT(idx >= 0 && idx < MAX_GUID, "Not a valid index - id: %d", id);
		const GUID& gid = _ids[idx];
		DialogItem& item = _items[gid.index];
		XASSERT(item.type == GIT_TEXT, "The GUI item %d is not a text item", id);
		// FIXME: workaround currently
		int len = strlen(text);
		XASSERT(len <= item.tmp, "Sorry but text is longer than the original version");
		v2 size = font::calculateSize(_font, text, 2);
		int sx = x -size.x * 0.5f;
		int sy = y -size.y * 0.5f;
		item.num = len;
		updateTextVertices(item.index, text, sx, sy);
	}

	// -------------------------------------------------------
	// Update formatted text
	// -------------------------------------------------------
	void GUIDialog::updateTextFormatted(int id, const char* format, ...) {
		va_list args;
		va_start(args, format);
		char buffer[1024];
		memset(buffer, 0, sizeof(buffer));
		int written = vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, format, args);
		updateText(id, buffer);
	}

	// -------------------------------------------------------
	// Update text
	// -------------------------------------------------------
	void GUIDialog::updateText(int id,const char* text) {	
		int idx = getIndexByID(id);
		XASSERT(idx >= 0 && idx < MAX_GUID, "Not a valid index - id: %d", id);
		const GUID& gid = _ids[idx];
		DialogItem& item = _items[gid.index];
		//XASSERT(item.type == GIT_TEXT, "The GUI item %d is not a text item", id);
		// FIXME: workaround currently
		int len = strlen(text);
		XASSERT(len <= item.tmp, "Sorry but text is longer than the original version");
		v2 size = font::calculateSize(_font, text, 2);
		int sx = -size.x * 0.5f;
		int sy = -size.y * 0.5f;
		item.num = len;
		updateTextVertices(item.index, text, sx, sy);
	}

	void GUIDialog::setVisible(ID id, bool visible) {
		int idx = getIndexByID(id);
		XASSERT(idx >= 0 && idx < MAX_GUID, "Not a valid index - id: %d", id);
		const GUID& gid = _ids[idx];
		DialogItem& item = _items[gid.index];
		item.visible = visible;
	}

	// -------------------------------------------------------
	// On button
	// -------------------------------------------------------
	int GUIDialog::onButton(int button,int x,int y,bool down) {
		for (uint32_t i = 0; i < _items.size(); ++i) {
			const DialogItem& item = _items[i];
			if (item.type == GIT_BUTTON || item.type == GIT_IMAGE_BUTTON) {
				Rect br = item.boundingRect;
				v2 p = item.pos;
				if (item.centered) {
					p.x = graphics::getScreenWidth() * 0.5f;
				}
				br.left += p.x;
				br.right += p.x;
				br.top += p.y;
				br.bottom += p.y;
				if (x >= br.left && x <= br.right && y <= br.top && y >= br.bottom) {
					return item.id;
				}
			}
		}
		return -1;
	}
	
	// -------------------------------------------------------
	// Set button texture
	// -------------------------------------------------------
	void GUIDialog::setButtonTexture(int id,const Rect& textureRect) {
		
	}

	// -------------------------------------------------------
	// add button 
	// -------------------------------------------------------
	GUID GUIDialog::addButton(int id, float x, float y, const char* text, const Rect& textureRect, const Color& textColor, const v2& textScale, bool centered, const Color& imageColor) {
		GUID& gid = _ids[_idIndex++];
		XASSERT(gid.id == -1, "The id %d is already in use", id);
		gid.id = id;
		gid.index = _items.size();
		v2 p = v2(x, y);
		if (centered) {
			p.x = graphics::getScreenWidth() * 0.5f;
		}
		DialogItem item;
		item.pos = p;
		item.centered = centered;
		item.color = imageColor;
		item.secondaryColor = textColor;
		item.rotation = 0.0f;
		item.scale = v2(1, 1);
		item.type = GIT_BUTTON;
		item.id = id;
		item.visible = true;
		item.index = _vertices.size();
		float w = textureRect.width();
		float h = textureRect.height();
		item.boundingRect = Rect(h * 0.5f, w * -0.5f, w, -h);
		DialogVertex v;
		v.offset = v2(0, 0);
		v.texture = math::buildTexture(textureRect);
		_vertices.push_back(v);

		v2 size = font::calculateSize(_font, text, 2);
		int sx = -size.x * 0.5f;
		int sy = -size.y * 0.5f;
		int len = addTextVertices(text, sx, sy);

		item.num = len + 1;
		_items.push_back(item);

		
		return gid;
		
	}

	// -------------------------------------------------------
	// Activate
	// -------------------------------------------------------
	void GUIDialog::activate() {
		m_SelectedInput = -1;			
		_current = INVALID_ID;
		_active = true;
	}

	// -------------------------------------------------------
	// Deactivate
	// -------------------------------------------------------
	void GUIDialog::deactivate() {
		_active = false;
	}

	// -------------------------------------------------------
	// get position
	// -------------------------------------------------------
	v2 GUIDialog::getPosition(int index) {
		const GUID& id = _ids[index];
		const DialogItem& item = _items[id.index];
		v2 p = item.pos;
		if (item.centered) {
			p.x = graphics::getScreenWidth() * 0.5f;
		}
		return p;
	}

	// -------------------------------------------------------
	// Render both nodes
	// -------------------------------------------------------
	void GUIDialog::render() {
		if (_active) {
			for (uint32_t i = 0; i < _items.size(); ++i) {
				const DialogItem& item = _items[i];
				if (item.visible) {
					int start = item.index;
					int end = start + item.num;					
					for (int j = start; j < end; ++j) {
						const DialogVertex& v = _vertices[j];						
						Color current = item.color;
						if (item.type == GIT_BUTTON && j > start) {
							current = item.secondaryColor;
						}
						_sprites->draw(item.pos + v.offset, v.texture, item.rotation, item.scale, current);
					}
				}
			}
		}
	}

	// -------------------------------------------------------
	// Update Mouse pos and set button textures if enabled
	// -------------------------------------------------------
	void GUIDialog::updateMousePos(const Vector2f& mousePos) {
	}

	// -------------------------------------------------------
	// update text vertices
	// -------------------------------------------------------
	void GUIDialog::updateTextVertices(int offset, const char* text,int sx, int sy) {
		int len = strlen(text);
		v2 size = font::calculateSize(_font, text, 2);
		for (int cnt = 0; cnt < len; ++cnt) {
			char c = text[cnt];
			const ds::Texture& t = _bitmapFont->get(c);
			float dimX = t.dim.x;// *item.scale.x;
			float dimY = t.dim.y;// *item.scale.y;
			_vertices[offset + cnt].texture = t;
			_vertices[offset + cnt].offset = v2(sx + dimX * 0.5f, sy + dimY * 0.5f);
			sx += dimX + 2;
		}
	}

	// -------------------------------------------------------
	// add text vertices
	// -------------------------------------------------------
	int GUIDialog::addTextVertices(const char* text,int sx, int sy) {
		int len = strlen(text);
		int padding = 2;
		v2 size = font::calculateSize(_font, text, 2);
		for (int cnt = 0; cnt < len; ++cnt) {
			char c = text[cnt];
			const ds::Texture& t = _bitmapFont->get(c);
			float dimX = t.dim.x;// *item.scale.x;
			float dimY = t.dim.y;// *item.scale.y;
			DialogVertex tv;
			tv.offset = v2(sx + dimX * 0.5f, sy + dimY * 0.5f);
			tv.texture = t;
			sx += dimX + padding;
			_vertices.push_back(tv);
		}
		return len;
	}

	// -------------------------------------------------------
	// tick
	// -------------------------------------------------------
	void GUIDialog::tick(float dt) {
		if (_active) {
			// handle transitions
			for (int i = 0; i < MAX_GUID; ++i) {
				if (_transitions[i].active) {
					DialogItem& item = _items[i];
					if (_transitions[i].timer < _transitions[i].ttl) {
						_transitions[i].timer += dt;
						if (_transitions[i].timer > _transitions[i].ttl) {
							_transitions[i].timer = _transitions[i].ttl;
						}
						item.pos = tweening::interpolate(_transitions[i].tweening, _transitions[i].start, _transitions[i].end, _transitions[i].timer, _transitions[i].ttl);
					}
					else {
						_transitions[i].active = false;
					}
				}
			}
			// handle color fades
			for (int i = 0; i < MAX_GUID; ++i) {
				if (_colorFades[i].active) {
					DialogItem& item = _items[i];
					if (_colorFades[i].timer < _colorFades[i].ttl) {
						_colorFades[i].timer += dt;
						if (_colorFades[i].timer > _colorFades[i].ttl) {
							_colorFades[i].timer = _colorFades[i].ttl;
						}
						item.color = tweening::interpolate(_colorFades[i].tweening, _colorFades[i].start, _colorFades[i].end, _colorFades[i].timer, _colorFades[i].ttl);
					}
					else {
						_colorFades[i].active = false;
					}
				}
			}
			// tick timers
			for (size_t i = 0; i < _timers.size(); ++i) {
				_timers[i].tick(dt);
			}
			// update timers if necessary
			char buffer[10];
			for (uint32_t i = 0; i < _items.size(); ++i) {
				const DialogItem& item = _items[i];
				if (item.type == GIT_TIMER) {
					const GameTimer& timer = _timers[item.tmp];
					if (timer.isDirty()) {
						string::formatTime(timer, buffer);
						updateTextVertices(item.index, buffer);
					}
				}
			}
			// handle hovering
			v2 mp = ds::input::getMousePosition();
			ID temp = INVALID_ID;
			for (uint32_t i = 0; i < _items.size(); ++i) {
				const DialogItem& item = _items[i];
				if (item.type == GIT_BUTTON || item.type == GIT_IMAGE_BUTTON) {
					Rect br = item.boundingRect;
					v2 p = item.pos;
					if (item.centered) {
						p.x = graphics::getScreenWidth() * 0.5f;
					}
					br.left += p.x;
					br.right += p.x;
					br.top += p.y;
					br.bottom += p.y;
					if (mp.x >= br.left && mp.x <= br.right && mp.y <= br.top && mp.y >= br.bottom) {
						temp = item.id;
					}
				}
			}
			if (temp != INVALID_ID) {
				if (temp != _current) {
					_current = temp;
					if (_hoverCallback) {
						_hoverCallback->entering(_current);
					}
				}
			}
			else {
				if (_current != INVALID_ID) {
					if (_hoverCallback) {
						_hoverCallback->leaving(_current);
					}
					_current = INVALID_ID;
				}
			}
		}
	}

	// -------------------------------------------------------
	// Set button hover
	// -------------------------------------------------------
	void GUIDialog::setButtonHover(const Rect& regularItem,const Rect& highlightItem) {
		m_ButtonItem = regularItem;
		m_ButtonItemSelected = highlightItem;
	}

	// -------------------------------------------------------
	// Find by id
	// -------------------------------------------------------
	int GUIDialog::getNextID() {
		int id = -1;
		for (int i = 0; i < MAX_GUID; ++i) {
			const GUID& gid = _ids[i];
			if (gid.id > id ) {
				id = gid.id;
			}
		}
		return id + 1;
	}

	// -------------------------------------------------------
	// contains item
	// -------------------------------------------------------
	bool GUIDialog::containsItem(int id) {
		for (size_t i = 0; i < _items.size(); ++i) {
			if (_items[i].id == id) {
				return true;
			}
		}
		return false;
	}
	// -------------------------------------------------------
	// Find by id
	// -------------------------------------------------------
	int GUIDialog::findFreeID() {
		for ( size_t i = 0; i < _items.size(); ++i ) {
			if (_items[i].id != i ) {
				return i;
			}
		}
		return _items.size();
	}

	// -------------------------------------------------------
	// swap
	// -------------------------------------------------------
	bool GUIDialog::swap(int currentIndex, int newIndex) {
		if (currentIndex >= 0 && currentIndex < 32 && newIndex >= 0 && newIndex < 32) {
			GUID current = _ids[currentIndex];
			_ids[currentIndex] = _ids[newIndex];
			_ids[newIndex] = current;
			return true;
		}
		return false;
	}

	// -------------------------------------------------------
	// remove
	// -------------------------------------------------------
	bool GUIDialog::remove(int id) {
		int idx = getIndexByID(id);
		if (idx != -1) {
			GUID& gid = _ids[idx];
			gid.id = -1;
			//gid.entryIndex = -1;
			gid.index = -1;
			return true;
		}
		return false;
	}

	// -------------------------------------------------------
	// save
	// -------------------------------------------------------
	bool GUIDialog::saveData(JSONWriter& jw) {	
		/*
		for (int i = 0; i < MAX_GUID; ++i) {
			const GUID& gid = _ids[i];
			if (gid.entryIndex != -1) {
				const GUIItem& gi = m_Items[gid.entryIndex];
				if (gi.type == GIT_IMAGE) {
					jw.startCategory("image");
					saveItem(jw, gid.id, gi);
					const GUIImage& image = _images[gid.index];
					jw.write("rect", image.texture.rect);
					jw.endCategory();
				}
				else if (gi.type == GIT_BUTTON) {
					jw.startCategory("button");
					saveItem(jw, gid.id, gi);
					const GUIButton& button = _buttons[gid.index];
					jw.write("rect", button.texture.rect);
					jw.write("text", button.text);
					jw.endCategory();
				}
				else if (gi.type == GIT_IMAGE_BUTTON) {
					jw.startCategory("image_button");
					saveItem(jw, gid.id, gi);
					const GUIImageButton& button = _imageButtons[gid.index];
					jw.write("rect", button.texture.rect);
					jw.endCategory();
				}
				else if (gi.type == GIT_TEXT) {
					jw.startCategory("text");
					saveItem(jw, gid.id, gi);
					const GUIText& text = _texts[gid.index];
					jw.write("text", text.text);
					jw.endCategory();
				}
				else if (gi.type == GIT_TIMER) {
					jw.startCategory("timer");
					saveItem(jw, gid.id, gi);
					jw.endCategory();
				}
				else if (gi.type == GIT_NUMBERS) {
					jw.startCategory("numbers");
					saveItem(jw, gid.id, gi);
					const GUINumber& number = _numbers[gid.index];
					jw.write("value", number.value);
					jw.write("length", number.length);
					jw.endCategory();
				}
			}
		}
		*/
		return true;
	}

	// -------------------------------------------------------
	// reload data
	// -------------------------------------------------------
	bool GUIDialog::reloadData(const JSONReader& reader) {
		return loadData(reader);
	}

	// -------------------------------------------------------
	// load data
	// -------------------------------------------------------
	bool GUIDialog::loadData(const JSONReader& reader) {
		clear();
		int cats[256];
		int num = reader.get_categories(cats, 256);
		for (int i = 0; i < num; ++i) {
			if (reader.matches(cats[i],"image")) {
				DialogItem item;
				int id = loadItem(cats[i], reader, &item);
				Rect r;
				reader.get(cats[i], "rect", &r);
				GUID gid = addImage(id, item.pos.x, item.pos.y, r, item.scale, item.centered, item.color);
			}			
			else if (reader.matches(cats[i], "image_button")) {
				DialogItem item;
				int id = loadItem(cats[i], reader, &item);
				Rect r;
				reader.get(cats[i], "rect", &r);
				GUID gid = addImageButton(id, item.pos.x, item.pos.y, r, item.centered);
			}
			else if (reader.matches(cats[i], "button")) {
				DialogItem item;
				int id = loadItem(cats[i], reader, &item);
				Rect r;
				reader.get(cats[i], "rect", &r);
				const char* label = reader.get_string(cats[i], "text");
				Color clr = Color::WHITE;
				reader.get(cats[i], "image_color", &clr);
				GUID gid = addButton(id, item.pos.x, item.pos.y, label, r, item.color, item.scale, item.centered, clr);
			}
			else if (reader.matches(cats[i], "text")) {
				DialogItem item;
				int id = loadItem(cats[i], reader, &item);
				const char* label = reader.get_string(cats[i], "text");
				GUID gid = addText(id, item.pos.x, item.pos.y, label, item.color, item.scale, item.centered);
			}
			else if (reader.matches(cats[i], "numbers")) {
				DialogItem item;
				int id = loadItem(cats[i], reader, &item);
				int value = 0;
				reader.get_int(cats[i],"value", &value);
				int length = 0;
				reader.get_int(cats[i], "length", &length);
				GUID gid = addNumber(id, item.pos, value, length, item.scale, item.color, item.centered);
			}
			else if (reader.matches(cats[i], "timer")) {
				DialogItem item;
				int id = loadItem(cats[i], reader, &item);
				GUID gid = addTimer(id, item.pos.x, item.pos.y, item.scale, item.color, item.centered);
			}
		}
		return true;
	}

	// -------------------------------------------------------
	// basic load item shared by all items
	// -------------------------------------------------------
	int GUIDialog::loadItem(int category, const JSONReader& reader, DialogItem* item) {
		int id = 0;
		reader.get_int(category,"id", &id);
		reader.get(category, "pos", &item->pos);
		reader.get(category, "color", &item->color);
		reader.get(category, "centered", &item->centered);
		reader.get(category, "scale", &item->scale);
		return id;
	}

	void GUIDialog::fadeColor(int id, const Color& start, const Color& end, float ttl) {
		int idx = getIndexByID(id);
		if (idx != -1) {
			GUIColorFade& gt = _colorFades[idx];
			gt.timer = 0.0f;
			gt.start = start;
			gt.ttl = ttl;
			DialogItem& item = _items[idx];
			gt.start = start;
			gt.end = end;
			item.color = start;
			gt.active = true;
		}
	}

	void GUIDialog::startTransition(int id, const v2& start, float ttl) {
		int idx = getIndexByID(id);
		if (idx != -1) {
			GUITransition& gt = _transitions[idx];
			gt.timer = 0.0f;
			gt.start = start;
			gt.ttl = ttl;
			DialogItem& item = _items[idx];
			gt.end = item.pos;
			item.pos = start;
			gt.active = true;
		}
	}
}
