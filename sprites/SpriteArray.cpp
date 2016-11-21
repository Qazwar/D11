#include "SpriteArray.h"
#include "core\log\Log.h"
#include "core\memory\DefaultAllocator.h"
#include "core\Common.h"
#include "core\math\math.h"
#include "core\base\Assert.h"

namespace ds {

	bool SpriteArray::verifySID(SID sid) {
		SpriteArrayIndex &in = indices[sid];
		if (in.index != USHRT_MAX) {
			return true;
		}
		LOG << "SID: " << sid << " is NOT valid - no valid index found";
		return false;
	}

	void SpriteArray::assertSID(SID sid) const {		
		XASSERT(sid < capacity, "ID %d out of range %d", sid, capacity);
		SpriteArrayIndex &in = indices[sid];
		XASSERT(in.index != UINT16_MAX, "Invalid index for %d", sid);
	}

	SID SpriteArray::create(const v2& pos, const Texture& r, float rotation, float scaleX, float scaleY, const Color& color, int type, int layer) {
		if (num + 1 > capacity) {
			allocate(capacity * 2 + 8);
		}
		ID id = 0;
		if (freeList.empty()) {
			id = current++;
		}
		else {
			id = freeList.back();
			freeList.pop_back();
		}
		SpriteArrayIndex &in = indices[id];
		in.index = num++;
		ids[in.index] = in.id;
		positions[in.index] = pos;
		scales[in.index] = v2(scaleX, scaleY);
		rotations[in.index] = rotation;
		textures[in.index] = r;
		colors[in.index] = color;
		timers[in.index] = 0.0f;
		types[in.index] = type;
		layers[in.index] = layer;
		previous[in.index] = pos;
		extents[in.index] = v2(0, 0);
		shapeTypes[in.index] = SST_NONE;
		return in.id;
	}

	void SpriteArray::remove(SID id) {
		SpriteArrayIndex& in = indices[id];
		XASSERT(in.index != UINT16_MAX,"Remove : Invalid index for %d",id);
		XASSERT(id < capacity,"Remove : ID %d out of range %d",id,capacity);
		if (num > 1) {
			ID lastID = ids[num-1];
			SpriteArrayIndex& lastIn = indices[lastID];
			if (lastID != in.index) {
				ids[in.index] = ids[lastIn.index];
				positions[in.index] = positions[lastIn.index];
				scales[in.index] = scales[lastIn.index];
				rotations[in.index] = rotations[lastIn.index];
				textures[in.index] = textures[lastIn.index];
				colors[in.index] = colors[lastIn.index];
				timers[in.index] = timers[lastIn.index];
				types[in.index] = types[lastIn.index];
				layers[in.index] = layers[lastIn.index];
				previous[in.index] = previous[lastIn.index];
				extents[in.index] = extents[lastIn.index];
				shapeTypes[in.index] = shapeTypes[lastIn.index];
				lastIn.index = in.index;
			}			
		}
		--num;
		in.index = UINT16_MAX;
		freeList.push_back(id);
	}

	void SpriteArray::allocate(uint16_t size) {
		if (size > capacity) {
			int sz = size * (sizeof(SpriteArrayIndex) + sizeof(SID) + sizeof(v2) + sizeof(v2) + sizeof(float) + sizeof(Texture) + sizeof(Color) + sizeof(float) + sizeof(uint16_t) + sizeof(uint16_t) + 2 * sizeof(v2) + sizeof(SpriteShapeType));
			char* b = (char*)ALLOC(sz);
			capacity = size;
			indices = (SpriteArrayIndex*)b;
			ids = (SID*)(indices + size);
			positions = (v2*)(ids + size);
			scales = (v2*)(positions + size);
			rotations = (float*)(scales + size);
			textures = (ds::Texture*)(rotations + size);
			colors = (ds::Color*)(textures + size);
			timers = (float*)(colors + size);
			types = (uint16_t*)(timers + size);
			layers = (uint16_t*)(types + size);
			previous = (v2*)(layers + size);
			extents = (v2*)(previous + size);
			shapeTypes = (SpriteShapeType*)(extents + size);
			if (buffer != 0) {
				memcpy(indices, buffer, num * sizeof(SpriteArrayIndex));
				int index = num * sizeof(SpriteArrayIndex);
				memcpy(ids, buffer + index, num * sizeof(SID));
				index += num * sizeof(SID);
				memcpy(positions, buffer + index, num * sizeof(v2));
				index += num * sizeof(v2);
				memcpy(scales, buffer + index, num * sizeof(v2));
				index += num * sizeof(v2);
				memcpy(rotations, buffer + index, num * sizeof(float));
				index += num * sizeof(float);
				memcpy(textures, buffer + index, num * sizeof(Texture));
				index += num * sizeof(Texture);
				memcpy(colors, buffer + index, num * sizeof(Color));
				index += num * sizeof(Color);
				memcpy(timers, buffer + index, num * sizeof(float));
				index += num * sizeof(float);
				memcpy(types, buffer + index, num * sizeof(uint16_t));
				index += num * sizeof(uint16_t);
				memcpy(layers, buffer + index, num * sizeof(uint16_t));
				index += num * sizeof(uint16_t);
				memcpy(previous, buffer + index, num * sizeof(v2));
				index += num * sizeof(v2);
				memcpy(extents, buffer + index, num * sizeof(v2));
				index += num * sizeof(v2);
				memcpy(shapeTypes, buffer + index, num * sizeof(SpriteShapeType));
				for (int i = num; i < capacity; ++i) {
					indices[i].id = i;
					indices[i].index = UINT16_MAX;
				}
				DEALLOC(buffer);
			}
			buffer = b;
		}
	}

	void SpriteArray::debug() {
		for (int i = 0; i < num; ++i) {
			LOG << i << " : id: " << ids[i] << " type: " << types[i] << " pos: " << positions[i];
		}
	}

	void SpriteArray::debug(SID sid) {
		SpriteArrayIndex &in = indices[sid];
		assertSID(sid);
		LOG << "id      : " << sid;
		LOG << "index   : " << in.index;
		LOG << "position: " << positions[in.index];
		LOG << "scale   : " << scales[in.index];
		LOG << "rotation: " << RADTODEG(rotations[in.index]);
		LOG << "color   : " << colors[in.index];
		LOG << "type    : " << types[in.index];
		LOG << "layer   : " << layers[in.index];
		LOG << "previous: " << previous[in.index];
		LOG << "extents : " << extents[in.index];
		LOG << "shape   : " << shapeTypes[in.index];
	}
	
	void SpriteArray::setPosition(SID sid, const v2& pos) {
		SpriteArrayIndex &in = indices[sid];
		assertSID(sid);
		positions[in.index] = pos;
	}

	const v2& SpriteArray::getPosition(SID sid) {
		SpriteArrayIndex &in = indices[sid];
		assertSID(sid);
		return positions[in.index];
	}

	void SpriteArray::setScale(SID sid, float sx, float sy) {
		SpriteArrayIndex &in = indices[sid];
		assertSID(sid);
		scales[in.index] = v2(sx,sy);
	}

	void SpriteArray::scale(SID sid, const v2& scale) {
		SpriteArrayIndex &in = indices[sid];
		assertSID(sid);
		scales[in.index] = scale;
	}

	void SpriteArray::setColor(SID sid, const Color& clr) {
		SpriteArrayIndex &in = indices[sid];
		assertSID(sid);
		colors[in.index] = clr;
	}

	void SpriteArray::setAlpha(SID sid, float alpha) {
		SpriteArrayIndex &in = indices[sid];
		assertSID(sid);
		colors[in.index].a = alpha;
	}

	void SpriteArray::rotate(SID sid, float angle) {
		SpriteArrayIndex &in = indices[sid];
		assertSID(sid);
		if ( angle > TWO_PI ) {
			angle -= TWO_PI;
		}
		rotations[in.index] = angle;
	}

	float SpriteArray::getRotation(SID sid) {
		SpriteArrayIndex &in = indices[sid];
		assertSID(sid);
		return rotations[in.index];
	}

	bool SpriteArray::get(SID sid,Sprite* ret) {
		SpriteArrayIndex &in = indices[sid];
		assertSID(sid);
		ret->id = sid;
		ret->position = positions[in.index];
		ret->scale = scales[in.index];
		ret->rotation = rotations[in.index];
		ret->texture = textures[in.index];
		ret->color = colors[in.index];
		ret->type = types[in.index];
		ret->layer = layers[in.index];
		return true;
	}

	void SpriteArray::set(SID sid, const Sprite& sprite) {
		SpriteArrayIndex &in = indices[sid];
		assertSID(sid);
		ids[in.index] = sid;
		positions[in.index] = sprite.position;
		scales[in.index] = sprite.scale;
		rotations[in.index] = sprite.rotation;
		textures[in.index] = sprite.texture;
		colors[in.index] = sprite.color;
		types[in.index] = sprite.type;
		layers[in.index] = sprite.layer;
	}
		
}