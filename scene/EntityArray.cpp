#include "EntityArray.h"
#include "..\utils\Log.h"

namespace ds {

	/*
	EntityArrayIndex* indices;
	ID* ids;
	v3* positions;
	v3* scales;
	v3* rotations;
	Color* colors;
	float* timers;
	uint16_t* types;
	Texture* textures;
	Mesh** meshes;
	mat4* worlds;
	ID* parents;
	DrawMode* drawModes;
	RID* materials;
	int* staticIndices;
	bool* active;
	*/
	void EntityArray::allocate(uint16_t size) {
		if (size > capacity) {
			int sum = sizeof(EntityArrayIndex) + sizeof(ID) + sizeof(v3) + sizeof(v3) + sizeof(v3) + sizeof(Color) + sizeof(float);
			sum += sizeof(uint16_t) + sizeof(Texture) + sizeof(Mesh*) + sizeof(mat4) + sizeof(ID) + sizeof(DrawMode) + sizeof(RID) + sizeof(int) + sizeof(bool) + sizeof(bool);
			int sz = size * sum;
			char* b = (char*)ALLOC(sz);			
			indices = (EntityArrayIndex*)b;
			ids = (ID*)(indices + size);
			positions = (v3*)(ids + size);
			scales = (v3*)(positions + size);
			rotations = (v3*)(scales + size);
			colors = (Color*)(rotations + size);
			timers = (float*)(colors + size);
			types = (uint16_t*)(timers + size);
			textures = (Texture*)(types + size);
			meshes = (Mesh**)(textures + size);
			worlds = (mat4*)(meshes + size);
			parents = (ID*)(worlds + size);
			drawModes = (DrawMode*)(parents + size);
			materials = (RID*)(drawModes + size);
			staticIndices = (int*)(materials + size);
			active = (bool*)(staticIndices + size);
			dirty = (bool*)(active + size);
			if (buffer != 0) {
				memcpy(indices, buffer, num * sizeof(EntityArrayIndex));
				int index = capacity * sizeof(EntityArrayIndex);
				memcpy(ids, buffer + index, num * sizeof(ID));
				index += capacity * sizeof(ID);
				memcpy(positions, buffer + index, num * sizeof(v3));
				index += capacity * sizeof(v3);
				memcpy(scales, buffer + index, num * sizeof(v3));
				index += capacity * sizeof(v3);
				memcpy(rotations, buffer + index, num * sizeof(v3));
				index += capacity * sizeof(v3);
				memcpy(colors, buffer + index, num * sizeof(Color));
				index += capacity * sizeof(Color);
				memcpy(timers, buffer + index, num * sizeof(float));
				index += capacity * sizeof(float);
				memcpy(types, buffer + index, num * sizeof(uint16_t));
				index += capacity * sizeof(uint16_t);
				memcpy(textures, buffer + index, num * sizeof(Texture));
				index += capacity * sizeof(Texture);
				memcpy(meshes, buffer + index, num * sizeof(Mesh*));
				index += capacity * sizeof(Mesh*);
				memcpy(worlds, buffer + index, num * sizeof(mat4));
				index += capacity * sizeof(mat4);
				memcpy(parents, buffer + index, num * sizeof(ID));
				index += capacity * sizeof(ID);
				memcpy(drawModes, buffer + index, num * sizeof(DrawMode));
				index += capacity * sizeof(DrawMode);
				memcpy(materials, buffer + index, num * sizeof(RID));
				index += capacity * sizeof(RID);
				memcpy(staticIndices, buffer + index, num * sizeof(int));
				index += capacity * sizeof(int);
				memcpy(active, buffer + index, num * sizeof(bool));
				index += capacity * sizeof(bool);
				memcpy(dirty, buffer + index, num * sizeof(bool));
				index += capacity * sizeof(bool);
				for (int i = num; i < size; ++i) {
					indices[i].id = i;
					indices[i].index = UINT16_MAX;
				}
				DEALLOC(buffer);
			}
			capacity = size;
			buffer = b;
		}
	}

	ID EntityArray::create(const v3& pos, Mesh* m, const v3& scale,const v3& rotation, RID material, const Color& color) {
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
		mat4 rotY = matrix::mat4RotationY(rotation.y);
		mat4 rotX = matrix::mat4RotationX(rotation.x);
		mat4 rotZ = matrix::mat4RotationZ(rotation.z);
		mat4 t = matrix::mat4Transform(pos);
		mat4 s = matrix::mat4Scale(scale);
		EntityArrayIndex &in = indices[id];
		in.index = num++;
		ids[in.index] = in.id;
		positions[in.index] = pos;
		scales[in.index] = scale;
		rotations[in.index] = rotation;		
		colors[in.index] = color;
		timers[in.index] = 0.0f;
		types[in.index] = 0;
		meshes[in.index] = m;
		worlds[in.index] = rotZ * rotY * rotX * s * t;
		parents[in.index] = INVALID_ID;
		drawModes[in.index] = DrawMode::TRANSFORM;
		materials[in.index] = material;
		staticIndices[in.index] = -1;
		active[in.index] = true;
		dirty[in.index] = false;
		return in.id;
	}

	ID EntityArray::create(const v2& pos, const Texture& t, const v2& scale, const float rotation, RID material, const Color& color) {
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
		EntityArrayIndex &in = indices[id];
		in.index = num++;
		ids[in.index] = in.id;
		positions[in.index] = v3(pos,0.0f);
		scales[in.index] = v3(scale,0.0f);
		rotations[in.index] = v3(0.0f,0.0f,rotation);
		colors[in.index] = color;
		timers[in.index] = 0.0f;
		types[in.index] = 0;
		textures[in.index] = t;
		meshes[in.index] = 0;
		worlds[in.index] = matrix::m4identity();
		parents[in.index] = INVALID_ID;
		drawModes[in.index] = DrawMode::TRANSFORM;
		materials[in.index] = material;
		staticIndices[in.index] = -1;
		active[in.index] = true;
		dirty[in.index] = false;
		return in.id;
	}

	bool EntityArray::contains(ID id) const {
		EntityArrayIndex &in = indices[id];
		if (in.index != USHRT_MAX) {
			return true;
		}
		LOG << "ID: " << id << " is NOT valid - no valid index found";
		return false;
	}

	int EntityArray::getIndex(ID id) const {
		EntityArrayIndex &in = indices[id];
		if (in.index != USHRT_MAX) {
			return in.index;
		}
		LOG << "ID: " << id << " is NOT valid - no valid index found";
		return -1;
	}

	void EntityArray::updateWorld(ID id) {
		EntityArrayIndex &in = indices[id];
		if (in.index != USHRT_MAX) {
			mat4 rotY = matrix::mat4RotationY(rotations[in.index].y);
			mat4 rotX = matrix::mat4RotationX(rotations[in.index].x);
			mat4 rotZ = matrix::mat4RotationZ(rotations[in.index].z);
			mat4 t = matrix::mat4Transform(positions[in.index]);
			mat4 s = matrix::mat4Scale(scales[in.index]);
			worlds[in.index] = rotZ * rotY * rotX * s * t;
		}
	}

	void EntityArray::setDrawMode(ID id, DrawMode mode) {
		EntityArrayIndex &in = indices[id];
		drawModes[in.index] = mode;
	}

	void EntityArray::setStaticIndex(ID id, int idx) {
		EntityArrayIndex &in = indices[id];
		staticIndices[in.index] = idx;
	}

	const mat4& EntityArray::getWorld(ID id) const {
		EntityArrayIndex &in = indices[id];
		return worlds[in.index];
	}

	void EntityArray::remove(ID id) {
		EntityArrayIndex& in = indices[id];
		//XASSERT(in.index != UINT16_MAX, "Invalid index for %d", id);
		//XASSERT(id < capacity, "ID %d out of range %d", id, capacity);
		if (num > 1) {
			int last = num - 1;
			ID lastID = ids[last];
			EntityArrayIndex& lastIn = indices[lastID];
			if (last != in.index) {
				ids[in.index] = ids[lastIn.index];
				positions[in.index] = positions[lastIn.index];
				scales[in.index] = scales[lastIn.index];
				rotations[in.index] = rotations[lastIn.index];
				colors[in.index] = colors[lastIn.index];
				timers[in.index] = timers[lastIn.index];
				types[in.index] = types[lastIn.index];
				textures[in.index] = textures[lastIn.index];
				meshes[in.index] = meshes[lastIn.index];
				worlds[in.index] = worlds[lastIn.index];
				parents[in.index] = parents[lastIn.index];
				drawModes[in.index] = drawModes[lastIn.index];
				materials[in.index] = materials[lastIn.index];
				staticIndices[in.index] = staticIndices[lastIn.index];
				active[in.index] = active[lastIn.index];
				dirty[in.index] = dirty[lastIn.index];
			}
			lastIn.index = in.index;
		}
		in.index = UINT16_MAX;
		freeList.push_back(id);
		--num;
	}
}