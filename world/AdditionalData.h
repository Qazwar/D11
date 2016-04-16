#pragma once
#include "..\sprites\Sprite.h"
#include "..\lib\collection_types.h"

namespace ds {

	struct AdditionalDataHeader {
		int index;
		int size;
		bool used;
		SID sid;
		int identifier;
	};

	class AdditionalData {

	public:
		AdditionalData() {}
		~AdditionalData() {}
		void* attach(SID sid, int size,int identifier);
		void remove(SID sid);
		void* get(SID sid);
		bool contains(SID sid);
		void debug();
		void save(const ReportWriter& writer);
	private:
		int find_free_header(int size);
		int find_header(SID sid);
		CharBuffer data;
		Array<AdditionalDataHeader> headers;
	};

}