#pragma once
#include "..\Common.h"
#include "..\renderer\render_types.h"
#include "..\renderer\MeshBuffer.h"
#include "ActionEventBuffer.h"

namespace ds {

	struct EntityArrayIndex {
		ID id;
		uint16_t index;
	};

	struct EntityArray {
	
		uint16_t num;
		uint16_t capacity;
		EntityArrayIndex* indices;

		ID* ids;
		v3* positions;
		v3* scales;
		v3* rotations;
		Color* colors;
		float* timers;
		uint16_t* types;
		Mesh** meshes;
		mat4* worlds;
		ID* parents;
		DrawMode* drawModes;
		RID* materials;
		int* staticIndices;
		bool* active;
		bool* dirty;
		char* buffer;

		ID current;
		Array<ID> freeList;

		EntityArray() : num(0), capacity(0), buffer(0), current(0) {
			allocate(256);
			clear();
		}

		void clear() {
			if (buffer != 0) {
				for (unsigned short i = 0; i < capacity; ++i) {
					indices[i].id = i;
					indices[i].index = UINT16_MAX;
				}				
				freeList.clear();
			}
			num = 0;
			current = 0;
		}

		void allocate(uint16_t size);

		bool contains(ID id) const;

		void remove(ID id);

		ID create(const v3& pos, Mesh* m, const v3& scale, const v3& rotation, RID material, const Color& color);

		void setDrawMode(ID id, DrawMode mode);

		void setStaticIndex(ID id, int idx);

		int getIndex(ID id) const;

		const mat4& getWorld(ID id) const;

		void updateWorld(ID id);
	};

}