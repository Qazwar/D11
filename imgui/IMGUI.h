#pragma once
#include <Vector.h>
#include <Point.h>
#include "..\renderer\BitmapFont.h"
#include "..\renderer\render_types.h"
#include "..\lib\collection_types.h"
#include "..\utils\Color.h"
#include "..\resources\ResourceDescriptors.h"

namespace gui {

	enum GUISetting {
		GS_LABELSIZE,
		GS_LINE_HEIGHT,
		GS_ALPHA,
		GS_EOL
	};
	
	class AbstractComponentModel {

	public:
		AbstractComponentModel() : _selected(-1) {}
		bool hasSelection() const {
			return _selected != -1;
		}
		void select(int idx) {
			_selected = idx;
		}
		bool isSelected(int idx) const {
			return idx == _selected;
		}
		int getSelection() const {
			return _selected;
		}
		virtual size_t size() const = 0;
		virtual const char* getLabel(int idx) const = 0;
		virtual void clear() = 0;
		virtual void remove(int idx) = 0;
	protected:
		int _selected;
	};

	template<class T>
	class ComponentModel : public AbstractComponentModel {

		struct ModelEntry {
			char name[32];
			T value;
		};

		typedef ds::Array<ModelEntry> Entries;

	public:
		ComponentModel() : AbstractComponentModel() {}
		void add(const char* label, const T& t) {
			ModelEntry entry;
			strcpy_s(entry.name, 32, label);
			entry.value = t;
			_entries.push_back(entry);
		}
		size_t size() const {
			return _entries.size();
		}
		const char* getLabel(int idx) const {
			return _entries[idx].name;
		}
		const T& getValue(int idx) const {
			return _entries[idx].value;
		}
		const T& getSelectedValue() const {
			assert(hasSelection());
			return _entries[_selected].value;
		}
		void clear() {
			_entries.clear();
			_selected = -1;
		}
		void remove(int idx) {
			_entries.remove(_entries.begin() + idx);
			_selected = -1;
		}
		void swap(int currentIndex, int newIndex) {
			if (currentIndex >= 0 && currentIndex < size() && newIndex >= 0 && newIndex < size()) {
				ModelEntry current = _entries[currentIndex];
				ModelEntry next = _entries[newIndex];
				_entries[newIndex] = current;
				_entries[currentIndex] = next;
			}
		}
	private:
		Entries _entries;
		
	};

	enum GUIDisplayMode {
		GDM_EDITOR,
		GDM_FLOATING
	};

	void initialize(const ds::IMGUIDescriptor& descriptor, bool editorMode = false);

	void switchEditorMode(bool editorMode);

	void start(int id,v2* startPos,bool forcePosition = false);

	void begin();

	bool begin(const char* header, int* state);

	void beginModal(const char* header);	

	void Header(const char* text);

	void beginGroup();

	void endGroup();

	void Text(const char* fmt, ...);

	void Label(const char* text);

	void Label(const char* text, const char* fmt, ...);

	bool Button(const char* label);

	int ButtonBar(const ds::Array<const char*>& entries);

	void CheckBox(const char* label,bool* selected);

	void InputFloat(const char* label, float* v);

	bool Input(const char* label, char* str,int maxLength);

	bool Input(char* str, int maxLength);

	void InputFloat(const char* label, float* v, float minValue, float maxValue, float step);

	void InputInt(const char* label, int* v);

	void InputInt(const char* label, uint32_t* v);

	void InputInt(const char* label, int* v,int minValue, int maxValue, int step);

	void InputVec2(const char* label, v2* v);

	void Input(const char* label, p2i* v);

	void InputVec3(const char* label, v3* v);

	void InputRect(const char* label, ds::Rect* v);

	void InputColor(const char* label, ds::Color* v);

	void ColorSlider(const char* label, ds::Color* v,int* state);

	void ComboBox(const ds::Array<const char*>& entries, int* selected, int *offset, int max = 5);

	void ComboBox(AbstractComponentModel* model,int *offset,int max = 5);

	void DropDownBox(const ds::Array<const char*>& entries, int* selected, int* state);

	void DropDownBox(const char** entries,int num, int* selected, int* state);

	void DropDownBox(AbstractComponentModel* model, int* state);

	void Histogram(float* values, int num, float minValue, float maxValue, float step);

	void Diagram(const char* label, float* values, int num, float minValue, float maxValue, float step);

	void Diagram(float* values, int num, float minValue, float maxValue, float step);

	int ActionBar(const ds::Array<const char*>& entries);

	void Separator();

	void ProgressBar(const char* label,float fraction);

	// simple value helper methods
	// will be converted to Label(label,...)
	void Value(const char* label, int v);

	void Value(const char* label, float v);

	void Value(const char* label, const v2& v);

	void Value(const char* label, const v3& v);

	void Value(const char* label, const ds::Color& v);

	void Value(const char* label, const char* text);

	void Image(const char* label, const ds::Rect& r, int textureID, const ds::Color& clr = ds::Color::WHITE);

	// Slider
	void Slider(const char* label, float* v, float minValue, float maxValue, float step);

	void sendKey(unsigned char c);

	void sendSpecialKey(WPARAM wParam);

	void debugWindow();

	void end();

	void shutdown();

	void endFrame();

	// gui settings
	void PushSetting(GUISetting setting, float value);

	void PopSetting(GUISetting setting);

	class InputDialog {

	public:
		InputDialog();
		~InputDialog() {}
		int show(const char* header,const char* label);
		int showEmbedded(const char* header, const char* label);
		const char* getText() const;
	private:
		bool _active;
		char _text[32];
		int _button;
	};

}
