#pragma once
#include "render_types.h"
#include "..\lib\collection_types.h"
#include "..\io\DataFile.h"
#include "..\utils\StaticHash.h"

namespace ds {

	const Texture EMPTY_TEXTURE = Texture(0, 0, 1024, 1024);

	class SpriteSheet : public JSONAssetFile {

		struct SheetEntry {
			StaticHash hash;
			Texture texture;
		};

	public:
		SpriteSheet(const char* fileName);
		~SpriteSheet();
		void add(const char* name, const Rect& r);
		const Texture& get(const char* name) const;
		const Texture& get(const StaticHash& sid) const;
		int findIndex(const char* name) const;
		int findIndex(const StaticHash& sid) const;
		const Texture& get(int index) const;
		bool loadData(const JSONReader& loader);
		bool reloadData(const JSONReader& loader);
	private:
		Array<SheetEntry> _entries;
	};

}