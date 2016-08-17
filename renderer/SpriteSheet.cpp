#include "SpriteSheet.h"
#include "..\utils\Log.h"
#include "..\utils\StringUtils.h"
#include "..\utils\Assert.h"

namespace ds {

	SpriteSheet::SpriteSheet(const char* fileName) : AssetFile(fileName) {
	}

	SpriteSheet::~SpriteSheet() {
	}

	void SpriteSheet::add(const char* name, const Rect& r) {
		SheetEntry entry;
		entry.hash = string::murmur_hash(name);
		entry.texture = Texture(r);
		_entries.push_back(entry);
	}

	const Texture& SpriteSheet::get(const char* name) const {
		int idx = findIndex(name);
		if (idx != -1) {
			return _entries[idx].texture;
		}
		return EMPTY_TEXTURE;
	}

	int SpriteSheet::findIndex(const char* name) const {
		IdString hash = string::murmur_hash(name);
		for (uint32_t i = 0; i < _entries.size(); ++i) {
			if (_entries[i].hash == hash) {
				return i;
			}
		}
		return -1;
	}

	const Texture& SpriteSheet::get(int index) const {
		XASSERT(index >= 0 && index < _entries.size(), "Invalid index");
		return _entries[index].texture;
	}

	bool SpriteSheet::loadData(const JSONReader& loader) {
		int categories[256];
		int num = loader.get_categories(categories, 256);
		for (int i = 0; i < num; ++i) {
			Rect r;
			loader.get(categories[i], "rect", &r);
			const char* sn = loader.get_string(categories[i], "name");
			add(sn, r);
		}
		return true;
	}

	bool SpriteSheet::reloadData(const JSONReader& loader) {
		int categories[256];
		int num = loader.get_categories(categories, 256);
		for (int i = 0; i < num; ++i) {
			Rect r;
			loader.get(categories[i], "rect", &r);
			const char* sn = loader.get_string(categories[i], "name");
			int idx = findIndex(sn);
			if (idx == -1) {
				add(sn, r);
			}
			else {
				SheetEntry& se = _entries[idx];
				se.texture = Texture(r);
			}
		}
		return true;
	}
}