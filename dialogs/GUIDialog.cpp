#include "GUIDialog.h"
#include "..\utils\Log.h"
#include "..\utils\StringUtils.h"
#include "..\utils\font.h"
#include "..\utils\Assert.h"
#include "..\resources\ResourceContainer.h"

namespace ds {
	GUIDialog::GUIDialog(const GUIDialogDescriptor& descriptor) {
		_font = descriptor.font;
		_sprites = res::getSpriteBuffer(descriptor.spriteBuffer);
		_state = 1;
		_offset = 0;
		_position = v2(1050, 690);
		_availableElements.push_back("Image");
		_availableElements.push_back("Text");
		_availableElements.push_back("Button");
		_availableElements.push_back("ImageButton");
		_availableElements.push_back("Number");
		_availableElements.push_back("Timer");
		_showAdd = false;
		_selectedElement = 0;
		_elementOffset = 0;
		_idIndex = 0;
		for (int i = 0; i < MAX_GUID; ++i) {
			_ids[i].id = -1;
			_ids[i].index = -1;
			_ids[i].entryIndex = -1;
			_transitions[i].id = -1;
		}
		_transitionCounter = 0;
		strcpy(_name, descriptor.file);
		sprintf_s(_jsonName, 128, "dialogs\\%s.json", _name);
	}

	// -------------------------------------------------------
	// clear
	// -------------------------------------------------------
	void GUIDialog::clear() {
		m_Items.clear();
		_buttons.clear();
		_texts.clear();
		_images.clear();
		_numbers.clear();
		_timers.clear();
		_idIndex = 0;
		for (int i = 0; i < MAX_GUID; ++i) {
			_ids[i].id = -1;
			_ids[i].index = -1;
			_ids[i].entryIndex = -1;
			_transitions[i].id = -1;
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
		return -1;
	}

	// -------------------------------------------------------
	// Init
	// -------------------------------------------------------
	/*
	void GUIDialog::init(const char* name,const DialogID& id,BitmapFont* bitmapFont) {	
		m_ID = id;
		m_BitmapFont = bitmapFont;
		m_Active = false;
		m_HashName = string::murmur_hash(name);
		m_SupportHover = false;
		m_SelectedInput = -1;
		strcpy(_name,name);
		sprintf_s(_jsonName, 128, "dialogs\\%s.json", _name);
		load();
	}
	*/

	// -------------------------------------------------------
	// Destructor
	// -------------------------------------------------------
	GUIDialog::~GUIDialog(void) {		
		_availableElements.clear();
	}

	// -------------------------------------------------------
	// create item
	// -------------------------------------------------------
	int GUIDialog::createItem(const v2& position,GUIItemType type,float scale,bool centered,const Color& color) {
		GUIItem item;
		item.pos = position;
		item.type = type;
		item.scale = scale;
		item.centered = centered;
		item.color = color;
		m_Items.push_back(item);
		return m_Items.size() - 1;
	}

	// -------------------------------------------------------
	// add number
	// -------------------------------------------------------
	GUID GUIDialog::addNumber(int id, const v2& position, int value, int length, float scale, const Color& color, bool centered) {
		GUID& gid = _ids[_idIndex++];
		XASSERT(gid.id == -1, "The id %d is already in use", id);
		gid.id = id;
		// add entry
		Vector2f p = position;
		gid.entryIndex = createItem(position, GIT_NUMBERS, scale, centered, color);
		GUINumber number;
		number.value = value;
		number.length = length;
		gid.index = _numbers.size();
		_numbers.push_back(number);
		return gid;
	}

	void GUIDialog::setNumber(int id, int value) {
		int idx = getIndexByID(id);
		XASSERT(idx != -1, "No matching GUI item for id: %d", id);
		const GUID& gid = _ids[idx];		
		GUINumber& number = _numbers[gid.index];
		number.value = value;
	}

	// -------------------------------------------------------
	// Add static image
	// -------------------------------------------------------
	GUID GUIDialog::addImage(int id, int x, int y, const Rect& textureRect,float scale, bool centered) {
		GUID& gid = _ids[_idIndex++];
		XASSERT(gid.id == -1, "The id %d is already in use", id);
		gid.id = id;
		// add entry
		Vector2f p = v2(x,y);
		gid.entryIndex = createItem(p, GIT_IMAGE, scale, centered);
		GUIImage image;
		image.texture = math::buildTexture(textureRect);
		gid.index = _images.size();
		_images.push_back(image);
		return gid;
	}

	// -------------------------------------------------------
	// update image
	// -------------------------------------------------------
	void GUIDialog::updateImage(int id, int x, int y, const Rect& textureRect, bool centered) {
		int idx = getIndexByID(id);
		const GUID& gid = _ids[idx];
		XASSERT(gid.id != -1, "No matching GUI item for %d", id);
		GUIItem& item = m_Items[gid.entryIndex];
		XASSERT(item.type == GIT_IMAGE,"This is not an image");
		GUIImage& image = _images[gid.index];
		item.centered = centered;
		item.pos = v2(x, y);
		image.texture = math::buildTexture(textureRect);
	}

	// -------------------------------------------------------
	// Add image link
	// -------------------------------------------------------
	GUID GUIDialog::addImageButton(int id,int x,int y,const Rect& textureRect,bool centered) {
		GUID& gid = _ids[_idIndex++];
		XASSERT(gid.id == -1, "The id %d is already in use", id);
		gid.id = id;
		// add entry
		Vector2f p = v2(x, y);
		gid.entryIndex = createItem(p, GIT_IMAGE_BUTTON, 1.0f, centered);
		GUIImageButton image;
		image.texture = math::buildTexture(textureRect);
		float w = textureRect.width();
		float h = textureRect.height();
		image.boundingRect = Rect(h * 0.5f, w * -0.5f, w, -h);
		gid.index = _imageButtons.size();
		_imageButtons.push_back(image);
		return gid;
	}

	// -------------------------------------------------------
	// add timer
	// -------------------------------------------------------
	GUID GUIDialog::addTimer(int id,int x, int y, float scale, const Color& color, bool centered) {
		GUID& gid = _ids[_idIndex++];
		XASSERT(gid.id == -1, "The id %d is already in use", id);
		gid.id = id;
		Vector2f p = v2(x, y);
		gid.entryIndex = createItem(p, GIT_TIMER, scale, centered, color);
		GameTimer timer;
		gid.index = _timers.size();
		_timers.push_back(timer);
		return gid;
	}

	// -------------------------------------------------------
	// Adds a text
	// -------------------------------------------------------
	GUID GUIDialog::addText(int id,int x,int y,const char* text,const Color& color,float scale,bool centered) {
		GUID& gid = _ids[_idIndex++];
		XASSERT(gid.id == -1, "The id %d is already in use", id);
		gid.id = id;
		Vector2f p = v2(x,y);
		gid.entryIndex = createItem(p, GIT_TEXT, scale, centered, color);
		GUIText guiText;
		strcpy(guiText.text, text);
		gid.index = _texts.size();
		_texts.push_back(guiText);
		return gid;
	}

	void GUIDialog::resetTimer(int id) {
		int idx = getIndexByID(id);
		const GUID& gid = _ids[idx];
		GUIItem& item = m_Items[gid.entryIndex];
		XASSERT(item.type == GIT_TIMER,"The GUI item %d is not a number",id);
		_timers[gid.index].reset();
	}

	void GUIDialog::startTimer(int id) {
		int idx = getIndexByID(id);
		const GUID& gid = _ids[idx];
		GUIItem& item = m_Items[gid.entryIndex];
		XASSERT(item.type == GIT_TIMER, "The GUI item %d is not a number", id);
		_timers[gid.index].start();
	}

	GameTimer* GUIDialog::getTimer(int id) {
		int idx = getIndexByID(id);
		const GUID& gid = _ids[idx];
		GUIItem& item = m_Items[gid.entryIndex];
		XASSERT(item.type == GIT_TIMER, "The GUI item %d is not a number", id);
		return &_timers[gid.index];
	}
	// -------------------------------------------------------
	// Update text
	// -------------------------------------------------------
	void GUIDialog::updateText(int id,int x,int y,const char* text,const Color& color,float scale,bool centered) {
		int idx = getIndexByID(id);
		const GUID& gid = _ids[idx];
		GUIItem& item = m_Items[gid.entryIndex];
		XASSERT(item.type == GIT_TEXT, "The GUI item %d is not a text item", id);
		GUIText& txt = _texts[gid.index];
		item.centered = centered;
		item.color = color;
		item.scale = scale;
		item.pos = v2(x,y);
		strcpy(txt.text, text);
	}

	// -------------------------------------------------------
	// Update text
	// -------------------------------------------------------
	void GUIDialog::updateText(int id,const char* text) {	
		int idx = getIndexByID(id);
		const GUID& gid = _ids[idx];
		GUIItem& item = m_Items[gid.entryIndex];
		XASSERT(item.type == GIT_TEXT, "The GUI item %d is not a text item", id);
		GUIText& txt = _texts[gid.index];
		strcpy(txt.text, text);
	}

	// -------------------------------------------------------
	// get text size
	// -------------------------------------------------------
	v2 GUIDialog::getTextSize(int id) {
		GUIItem* item = findByID(id);
		XASSERT(item != 0,"Cannot find text item");
		return item->size;
	}

	// -------------------------------------------------------
	// On button
	// -------------------------------------------------------
	int GUIDialog::onButton(int button,int x,int y,bool down) {
		if ( !m_Active ) {
			return -1;
		}
		for (int i = 0; i < MAX_GUID; ++i) {
			const GUID& gid = _ids[i];
			if ( gid.entryIndex != -1 ) {
				const GUIItem& item = m_Items[gid.entryIndex];
				if (item.type == GIT_BUTTON || item.type == GIT_IMAGE_BUTTON) {
					Rect br;
					if (item.type == GIT_BUTTON) {
						const GUIButton& button = _buttons[gid.index];
						br = button.boundingRect;
					}
					else  {
						const GUIImageButton& button = _imageButtons[gid.index];
						br = button.boundingRect;
					}
					v2 p = item.pos;
					if (item.centered) {
						p.x = graphics::getScreenWidth() * 0.5f;
					}
					br.left += p.x;
					br.right += p.x;
					br.top += p.y;
					br.bottom += p.y;
					if (x >= br.left && x <= br.right && y <= br.top && y >= br.bottom) {
						return gid.id;
					}
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
	GUID GUIDialog::addButton(int id,float x,float y, const char* text, const Rect& textureRect, const Color& textColor, float textScale, bool centered) {
		GUID& gid = _ids[_idIndex++];
		XASSERT(gid.id == -1, "The id %d is already in use", id);
		gid.id = id;
		// add entry
		Vector2f p = v2(x, y);
		gid.entryIndex = createItem(p, GIT_BUTTON, textScale, centered, textColor);
		GUIButton button;
		strcpy(button.text, text);
		button.texture = math::buildTexture(textureRect);
		float w = textureRect.width();
		float h = textureRect.height();
		button.boundingRect = Rect(h * 0.5f, w * -0.5f, w, -h);
		gid.index = _buttons.size();
		_buttons.push_back(button);
		return gid;
		
	}

	// -------------------------------------------------------
	// Activate
	// -------------------------------------------------------
	void GUIDialog::activate() {
		m_Active = true;		
		m_SelectedInput = -1;			
		if (_transitionCounter > 0) {
			_transitionMode = true;
			for (int i = 0; i < MAX_GUID; ++i) {
				_transitions[i].timer = 0.0f;
			}
		}
	}

	// -------------------------------------------------------
	// Deactivate
	// -------------------------------------------------------
	void GUIDialog::deactivate() {
		m_Active = false;
	}

	// -------------------------------------------------------
	// get position
	// -------------------------------------------------------
	v2 GUIDialog::getPosition(int index) {
		const GUID& id = _ids[index];
		const GUIItem& item = m_Items[id.entryIndex];
		v2 p = item.pos;
		if (item.centered) {
			p.x = graphics::getScreenWidth() * 0.5f;
		}
		if (_transitionMode) {
			if (_transitions[index].id != -1) {
				float norm = _transitions[index].timer / _transitions[index].ttl;				
				// float from left to position
				if (_transitions[index].typeBits == 1) {
					return lerp(v2(-200, item.pos.y), p, norm);
				}	
				// floar right to position
				if (_transitions[index].typeBits == 2) {
					return lerp(v2(1200, item.pos.y), p, norm);
				}
				if (_transitions[index].typeBits == 3) {
					return lerp(v2(p.x, 900.0f), p, norm);
				}
				if (_transitions[index].typeBits == 4) {
					return lerp(v2(p.x, -200.0f), p, norm);
				}
			}
		}
		return p;
	}
	// -------------------------------------------------------
	// Render both nodes
	// -------------------------------------------------------
	void GUIDialog::render() {
		_sprites->begin();
		//if ( m_Active ) {
			char buffer[32];
			for (int i = 0; i < MAX_GUID; ++i) {
				const GUID& id = _ids[i];
				if (id.entryIndex != -1) {
					const GUIItem& item = m_Items[id.entryIndex];
					v2 p = item.pos;
					if (item.type == GIT_NUMBERS) {
						const GUINumber& number = _numbers[id.index];
						string::formatInt(number.value, buffer, 32, number.length);
						v2 size = font::calculateSize(_font, buffer, 2, item.scale, item.scale);
						float ty = p.y - size.y * 0.5f;
						if (item.centered) {
							p.x = graphics::getScreenWidth() * 0.5f;
						}
						p += v2(size.x * -0.5f, -size.y * 0.5f);
						_sprites->drawText(_font, p.x, p.y, buffer, 2.0f, item.scale, item.scale, item.color);
					}
					else if (item.type == GIT_TEXT) {
						const GUIText& text = _texts[id.index];
						v2 size = font::calculateSize(_font, text.text, 2, item.scale, item.scale);
						float ty = p.y - size.y * 0.5f;
						if (item.centered) {
							p.x = graphics::getScreenWidth() * 0.5f;
						}
						p += v2(size.x * -0.5f, -size.y * 0.5f);
						_sprites->drawText(_font, p.x, p.y, text.text, 2.0f, item.scale, item.scale, item.color);
					}
					else if (item.type == GIT_TIMER) {
						const GameTimer& timer = _timers[id.index];
						sprintf_s(buffer, 32, "%02d:%02d", timer.getMinutes(), timer.getSeconds());
						v2 size = font::calculateSize(_font, buffer, item.scale);
						float ty = p.y - size.y * 0.5f;
						p += v2(size.x * -0.5f, -size.y * 0.5f);
						_sprites->drawText(_font, p.x, p.y, buffer, 2.0f, item.scale, item.scale, item.color);
					}
					else if (item.type == GIT_IMAGE) {
						const GUIImage& image = _images[id.index];
						v2 p = item.pos;
						if (item.centered) {
							p.x = graphics::getScreenWidth() * 0.5f;
						}
						_sprites->draw(p, image.texture, 0.0f, v2(item.scale, item.scale), item.color);
					}
					else if (item.type == GIT_IMAGE_BUTTON) {
						const GUIImageButton& image = _imageButtons[id.index];
						v2 p = item.pos;
						if (item.centered) {
							p.x = graphics::getScreenWidth() * 0.5f;
						}
						_sprites->draw(p, image.texture, 0.0f, v2(item.scale, item.scale), item.color);
					}
					else if (item.type == GIT_BUTTON) {
						const GUIButton& button = _buttons[id.index];
						v2 p = getPosition(i);
						//v2 p = item.pos;
						//if (item.centered) {
							//p.x = renderer::getScreenWidth() * 0.5f;
						//}
						_sprites->draw(p, button.texture);
						v2 size = font::calculateSize(_font, button.text, item.scale);
						float ty = p.y - size.y * 0.5f;
						p += v2(size.x * -0.5f, -size.y * 0.5f);
						_sprites->drawText(_font, p.x, p.y, button.text, 2.0f, item.scale, item.scale, item.color);
					}
				}
			}
		//}
			_sprites->end();
	}

	// -------------------------------------------------------
	// Update Mouse pos and set button textures if enabled
	// -------------------------------------------------------
	void GUIDialog::updateMousePos(const Vector2f& mousePos) {
		if ( m_SupportHover ) {
			int ret = onButton(0,mousePos.x,mousePos.y,false);	
			for ( size_t i = 0; i < m_Items.size(); ++i ) {
				//DialogButton* db = &m_Buttons[i];		
				//if ( db->id == ret && ret != m_Selected ) {
					//setButtonTexture(db->id,m_ButtonItemSelected);
				//}
			}
			if ( ret == -1 && m_Selected != -1 ) {
				//for ( size_t i = 0; i < m_Buttons.size(); ++i ) {
					//DialogButton* db = &m_Buttons[i];		
					//if ( m_Selected == db->id ) {
						//setButtonTexture(db->id,m_ButtonItem);
					//}
				//}
			}
			m_Selected = ret;
		}
	}

	// -------------------------------------------------------
	// tick
	// -------------------------------------------------------
	void GUIDialog::tick(float dt) {
		if (_transitionMode) {
			int cnt = 0;
			for (int i = 0; i < MAX_GUID; ++i) {
				if (_transitions[i].id != -1) {
					if (_transitions[i].timer < _transitions[i].ttl) {
						_transitions[i].timer += dt;
					}
					else {
						++cnt;
					}
				}
			}
			if (cnt == _transitionCounter) {
				_transitionMode = false;
			}
		}
		for (size_t i = 0; i < _timers.size(); ++i) {
			_timers[i].tick(dt);
		}
	}

	void GUIDialog::setTransition(int id, int type, float ttl) {
		int idx = getIndexByID(id);
		if (idx != -1) {
			_transitions[idx].id = id;
			_transitions[idx].timer = 0.0f;
			_transitions[idx].ttl = ttl;
			_transitions[idx].typeBits = type;
			++_transitionCounter;
		}
	}

	// -------------------------------------------------------
	// Set button hover
	// -------------------------------------------------------
	void GUIDialog::setButtonHover(const Rect& regularItem,const Rect& highlightItem) {
		m_SupportHover = true;
		m_ButtonItem = regularItem;
		m_ButtonItemSelected = highlightItem;
	}

	// -------------------------------------------------------
	// Find by id
	// -------------------------------------------------------
	GUIItem* GUIDialog::findByID(int id) {
		for ( size_t i = 0; i < m_Items.size(); ++i ) {
			GUIItem* gi = &m_Items[i];
			if ( gi->id == id ) {
				return gi;
			}
		}
		return 0;
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
		for (size_t i = 0; i < m_Items.size(); ++i) {
			if (m_Items[i].id == id) {
				return true;
			}
		}
		return false;
	}
	// -------------------------------------------------------
	// Find by id
	// -------------------------------------------------------
	int GUIDialog::findFreeID() {
		for ( size_t i = 0; i < m_Items.size(); ++i ) {
			GUIItem* gi = &m_Items[i];
			if ( gi->id != i ) {
				return i;
			}
		}
		return m_Items.size();
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
			gid.entryIndex = -1;
			gid.index = -1;
			return true;
		}
		return false;
	}

	// -------------------------------------------------------
	// show add new item dialog
	// -------------------------------------------------------
	void GUIDialog::showAddDialog() {
		if (_showAdd) {
			if (gui::begin("Add element", &_state)) {
				gui::ComboBox(_availableElements, &_selectedElement, &_elementOffset);
				gui::beginGroup();
				if (gui::Button("OK")) {
					if (_selectedElement != -1) {
						int id = getNextID();
						XASSERT(id != -1, "Cannot get a next ID");
						if (_selectedElement == 0) {
							GUID gid = addImage(id, 512, 384, Rect(0,0,50,50), true);
							addToModel(gid.id, GIT_IMAGE, "Image");
						}
						else if (_selectedElement == 1) {
							GUID gid = addText(id, 512, 384, "Text", Color::WHITE, 1.0f, true);
							addToModel(gid.id, GIT_TEXT, "Text");
						}
						else if (_selectedElement == 2) {
							GUID gid = addButton(id, 512, 384, "Text", Rect(0,0,50,50), Color::WHITE, 1.0f, true);
							addToModel(gid.id, GIT_BUTTON, "Button");
						}
						else if (_selectedElement == 3) {
							GUID gid = addImageButton(id, 512, 384, Rect(0,0,50,50),true);
							addToModel(gid.id, GIT_IMAGE_BUTTON, "ImageButton");
						}
						else if (_selectedElement == 4) {
							GUID gid = addNumber(id,v2(512, 384), 0, 3);
							addToModel(gid.id, GIT_NUMBERS, "Numbers");
						}
						else if (_selectedElement == 5) {
							GUID gid = addTimer(id,512, 384);
							addToModel(gid.id, GIT_TIMER, "Timer");
						}
					}
					_showAdd = false;
				}
				if (gui::Button("Cancel")) {
					_showAdd = false;
				}
				gui::endGroup();
			}
			gui::end();			
		}
	}

	// -------------------------------------------------------
	// add to model
	// -------------------------------------------------------
	void GUIDialog::addToModel(int id, GUIItemType type,const char* prefix) {
		char buffer[32];
		sprintf_s(buffer, 32, "%s %d", prefix, id);
		GUIModelItem hge;
		hge.id = id;
		hge.type = type;
		_model.add(buffer, hge);
	}

	// -------------------------------------------------------
	// show dialog
	// -------------------------------------------------------
	void GUIDialog::showDialog() {
		if (gui::begin(_name, &_state)) {
			gui::ComboBox(&_model, &_offset);
			gui::beginGroup();
			if (gui::Button("Save")) {
				save();
			}
			if (gui::Button("Load")) {
				load();
			}
			gui::endGroup();
			gui::beginGroup();
			if (gui::Button("Add")) {
				_showAdd = !_showAdd;
			}
			if (gui::Button("Remove")) {
				if (_model.hasSelection()) {
					const GUIModelItem& item = _model.getSelectedValue();
					if (remove(item.id)) {
						_model.remove(_model.getSelection());
					}
				}
			}
			if (gui::Button("Up")) {
				if (_model.hasSelection()) {
					int index = _model.getSelection();
					if (swap(index, index - 1)) {
						_model.swap(index, index - 1);
						_model.select(index - 1);
					}
				}
			}
			if (gui::Button("Down")) {
				if (_model.hasSelection()) {
					int index = _model.getSelection();
					if (swap(index, index + 1)) {
						_model.swap(index, index + 1);
						_model.select(index + 1);
					}
				}
			}
			gui::endGroup();
		}
		gui::end();

		showAddDialog();

		if (_model.hasSelection()) {
			if (gui::begin("GUI Element", &_state)) {
				GUIModelItem element = _model.getSelectedValue();
				int idx = getIndexByID(element.id);
				const GUID& gid = _ids[idx];
				if (gid.entryIndex != -1) {
					GUIItem* item = &m_Items[gid.entryIndex];
					char buffer[32];
					sprintf_s(buffer, 32, "ID: %d", gid.id);
					gui::Label(buffer);
					gui::InputVec2("Position", &item->pos);
					gui::InputFloat("Scale", &item->scale);
					gui::InputColor("Color", &item->color);
					gui::CheckBox("Centered", &item->centered);
					if (element.type == GIT_NUMBERS) {
						GUINumber* number = &_numbers[gid.index];
						gui::InputInt("Value", &number->value);
						gui::InputInt("Length", &number->length);
					}
					else if (element.type == GIT_IMAGE) {
						GUIImage* image = &_images[gid.index];
						Rect r = image->texture.rect;
						gui::InputRect("Texture", &r);
						image->texture = math::buildTexture(r);
					}
					else if (element.type == GIT_BUTTON) {
						GUIButton* button = &_buttons[gid.index];
						Rect r = button->texture.rect;
						gui::InputRect("Texture", &r);
						button->texture = math::buildTexture(r);
						gui::Input("Text", button->text, 32);
					}
					else if (element.type == GIT_IMAGE_BUTTON) {
						GUIImageButton* button = &_imageButtons[gid.index];
						Rect r = button->texture.rect;
						gui::InputRect("Texture", &r);
						button->texture = math::buildTexture(r);
					}
					else if (element.type == GIT_TEXT) {
						GUIText* text = &_texts[gid.index];
						gui::Input("Text", text->text, 32);
					}
				}
			}
			gui::end();
		}
	}
	/*
	void GUIDialog::saveItem(BinaryWriter& writer,int id,const GUIItem& item) {
		writer.write(id);
		writer.write(item.pos);
		writer.write(item.color);
		int cnt = 0;
		if (item.centered) {
			cnt = 1;
		}
		writer.write(cnt);
		writer.write(item.scale);
	}
	*/
	void GUIDialog::saveItem(JSONWriter& writer, int id, const GUIItem& item) {
		writer.write("id", id);
		writer.write("pos", item.pos);
		writer.write("color", item.color);
		writer.write("centered", item.centered);
		writer.write("scale", item.scale);
	}

	
	
	// -------------------------------------------------------
	// save
	// -------------------------------------------------------
	bool GUIDialog::saveData(JSONWriter& jw) {		
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
		return true;
	}

	bool GUIDialog::loadData(const JSONReader& reader) {
		clear();
		_model.clear();
		int cats[256];
		int num = reader.get_categories(cats, 256);
		for (int i = 0; i < num; ++i) {
			if (reader.matches(cats[i],"image")) {
				GUIItem item;
				int id = loadItem(cats[i], reader, &item);
				Rect r;
				reader.get(cats[i], "rect", &r);
				GUID gid = addImage(id, item.pos.x, item.pos.y, r, item.scale, item.centered);
				addToModel(gid.id, GIT_IMAGE, "Image");
			}
			else if (reader.matches(cats[i], "button")) {
				GUIItem item;
				int id = loadItem(cats[i], reader, &item);
				Rect r;
				reader.get(cats[i], "rect", &r);
				const char* label = reader.get_string(cats[i], "text");
				GUID gid = addButton(id, item.pos.x, item.pos.y, label, r, item.color, item.scale, item.centered);
				addToModel(gid.id, GIT_BUTTON, "Button");
			}
			else if (reader.matches(cats[i], "image_button")) {
				GUIItem item;
				int id = loadItem(cats[i], reader, &item);
				Rect r;
				reader.get(cats[i], "rect", &r);
				GUID gid = addImageButton(id, item.pos.x, item.pos.y, r, item.centered);
				addToModel(gid.id, GIT_IMAGE_BUTTON, "ImageButton");
			}
			else if (reader.matches(cats[i], "text")) {
				GUIItem item;
				int id = loadItem(cats[i], reader, &item);
				const char* label = reader.get_string(cats[i], "text");
				GUID gid = addText(id, item.pos.x, item.pos.y, label, item.color, item.scale, item.centered);
				addToModel(gid.id, GIT_TEXT, "Text");
			}
			else if (reader.matches(cats[i], "numbers")) {
				GUIItem item;
				int id = loadItem(cats[i], reader, &item);
				int value = 0;
				reader.get_int(cats[i],"value", &value);
				int length = 0;
				reader.get_int(cats[i], "length", &length);
				GUID gid = addNumber(id, item.pos, value, length, item.scale, item.color, item.centered);
				addToModel(gid.id, GIT_NUMBERS, "Number");
			}
			else if (reader.matches(cats[i], "timer")) {
				GUIItem item;
				int id = loadItem(cats[i], reader, &item);
				GUID gid = addTimer(id, item.pos.x, item.pos.y, item.scale, item.color, item.centered);
				addToModel(gid.id, GIT_TIMER, "Timer");
			}
		}
		return true;
	}

	int GUIDialog::loadItem(int category, const JSONReader& reader, GUIItem* item) {
		int id = 0;
		reader.get_int(category,"id", &id);
		reader.get_vec2(category, "pos", &item->pos);
		reader.get_color(category, "color", &item->color);
		reader.get(category, "centered", &item->centered);
		reader.get_float(category, "scale", &item->scale);
		return id;
	}
}
