#pragma once
#include <Vector.h>
#include "core\graphics\Color.h"
#include "..\renderer\render_types.h"
#include "Sprite.h"
#include "core\lib\collection_types.h"
#include "core\graphics\Texture.h"
#include <stdint.h>

namespace ds {

	struct SpriteArrayIndex {
		SID id;
		uint16_t index;
	};

	struct SpriteArray {

		uint16_t num;
		uint16_t capacity;
		SpriteArrayIndex* indices;
		SID* ids;
		v2* positions;
		v2* scales;
		float* rotations;
		Texture* textures;
		Color* colors;
		float* timers;
		uint16_t* types;
		uint16_t* layers;
		// physical
		v2* previous;
		v2* extents;
		SpriteShapeType* shapeTypes;
		char* buffer;

		ID current;
		Array<ID> freeList;

		SpriteArray() : num(0) , capacity(0) , buffer(0) , current(0) {			
			allocate(256);
			clear();
		}

		void clear() {
			if (buffer != 0) {
				for (unsigned short i = 0; i < capacity; ++i) {
					indices[i].id = i;
					indices[i].index = UINT16_MAX;
				}
				num = 0;
				current = 0;
				freeList.clear();
			}
		}

		bool verifySID(SID sid);

		void assertSID(SID sid) const;

		const int getType(SID sid) const {
			SpriteArrayIndex &in = indices[sid];
			assert(in.index != USHRT_MAX);
			return types[in.index];
		}

		void setType(SID sid,int type) {
			SpriteArrayIndex &in = indices[sid];
			assertSID(sid);
			types[in.index] = type;
		}

		const bool contains(SID sid) const {
			SpriteArrayIndex& in = indices[sid];
			return in.index != USHRT_MAX;
		}

		const int getIndex(SID sid) const {
			SpriteArrayIndex &in = indices[sid];
			assertSID(sid);
			return in.index;
		}

		const v2& getPosition(SID sid) const {
			SpriteArrayIndex &in = indices[sid];
			assertSID(sid);
			return positions[in.index];
		}

		const v2& getScale(SID sid) const {
			SpriteArrayIndex &in = indices[sid];
			assertSID(sid);
			return scales[in.index];
		}

		const float getRotation(SID sid) const {
			SpriteArrayIndex &in = indices[sid];
			assertSID(sid);
			return rotations[in.index];
		}

		const Texture& getTexture(SID sid) const {
			SpriteArrayIndex &in = indices[sid];
			assertSID(sid);
			return textures[in.index];
		}

		SID create(const v2& pos, const Texture& r, float rotation = 0.0f, float scaleX = 1.0f, float scaleY = 1.0f, const Color& color = Color::WHITE, int type = -1, int layer = 0);

		void attachCollider(SID sid, const v2& extent, SpriteShapeType shape = SST_CIRCLE) {
			SpriteArrayIndex &in = indices[sid];
			assertSID(sid);
			shapeTypes[in.index] = shape;
			extents[in.index] = extent;
		}

		void attachCollider(SID sid, SpriteShapeType shape = SST_CIRCLE) {
			const Texture& t = getTexture(sid);
			attachCollider(sid, t.dim, shape);
		}

		void remove(SID id);

		void allocate(uint16_t size);

		void debug();

		void debug(SID id);

		void setPosition(SID sid, const v2& pos);

		const v2& getPosition(SID sid);

		void setScale(SID sid, float sx, float sy);

		void scale(SID sid, const v2& scale);

		void setColor(SID sid, const Color& clr);

		void setAlpha(SID sid, float alpha);

		void rotate(SID sid, float angle);

		float getRotation(SID sid);

		bool get(SID sid,Sprite* ret);

		void set(SID sid, const Sprite& sprite);
	};


}
