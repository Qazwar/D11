#include "SpriteSheet.h"
#include "..\utils\Log.h"
#include "..\utils\StringUtils.h"
#include "..\utils\Assert.h"

namespace ds {

	SpriteSheet::SpriteSheet(const char* fileName) : JSONAssetFile(fileName) {
	}

	SpriteSheet::~SpriteSheet() {
	}

	void SpriteSheet::add(const char* name, const Rect& r) {
		SheetEntry entry;
		entry.hash = SID(name);
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

	const Texture& SpriteSheet::get(const StaticHash& sid) const {
		int idx = findIndex(sid);
		if (idx != -1) {
			return _entries[idx].texture;
		}
		return EMPTY_TEXTURE;
	}

	int SpriteSheet::findIndex(const char* name) const {
		StaticHash hash = SID(name);
		for (uint32_t i = 0; i < _entries.size(); ++i) {
			if (_entries[i].hash == hash) {
				return i;
			}
		}
		XASSERT(1 == 2, "No matching spritesheet found for '%s'", name);
		return -1;
	}

	int SpriteSheet::findIndex(const StaticHash& sid) const {
		for (uint32_t i = 0; i < _entries.size(); ++i) {
			if (_entries[i].hash == sid) {
				return i;
			}
		}
		XASSERT(1 == 2, "No matching spritesheet found for '%d'", sid);
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