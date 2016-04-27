#include "BlockArray.h"
#include <string.h>
#include "..\memory\DefaultAllocator.h"

BlockArray::BlockArray() : size(0), capacity(0), data(0) , total_capacity(0) {
}


BlockArray::~BlockArray() {
	if (data != 0) {
		//ds::gDefaultMemory->deallocate(data);
		DEALLOC(data);
	}
}

void BlockArray::init(int* sizes, int num) {
	assert(num < MAX_BLOCKS);
	for (int i = 0; i < num; ++i) {
		_sizes[i] = sizes[i];
	}
	_num_blocks = num;
}

bool BlockArray::resize(int new_size) {
	if (new_size > capacity) {
		new_size = capacity * 2 + new_size;
		int total = 0;
		for (int i = 0; i < _num_blocks; ++i) {
			total += _sizes[i];
		}
		
		int sz = new_size * total;
		//char* t = (char*)ds::gDefaultMemory->allocate(sz);
		char* t = (char*)ALLOC(sz);
		if (data != 0) {
			int offset = 0;
			int old_offset = 0;
			for (int i = 0; i < _num_blocks; ++i) {
				memcpy(t + offset, data + old_offset, size * _sizes[i]);
				offset += new_size * _sizes[i];
				old_offset += capacity * _sizes[i];
			}
			//ds::gDefaultMemory->deallocate(data);
			DEALLOC(data);
		}
		capacity = new_size;
		_indices[0] = 0;
		for (int i = 1; i < _num_blocks; ++i) {
			_indices[i] = _indices[i - 1] + _sizes[i- 1] * capacity;
		}
		data = t;
		return true;
	}
	return false;
}

void* BlockArray::get_ptr(int index) {
	return data + _indices[index];
}

void BlockArray::remove(int index) {
	int last = size - 1;
	for (int i = 0; i < _num_blocks; ++i) {
		char* dest = data + _indices[i] + index * _sizes[i];
		char* src = data + _indices[i] + last * _sizes[i];
		memcpy(dest, src, _sizes[i]);
	}
	--size;
}

void BlockArray::swap(int newIndex,int oldIndex) {
	for (int i = 0; i < _num_blocks; ++i) {
		char* dest = data + _indices[i] + newIndex * _sizes[i];
		char* src = data + _indices[i] + oldIndex * _sizes[i];
		memcpy(dest, src, _sizes[i]);
	}
}

namespace ds {

	ChannelArray::ChannelArray() : size(0), capacity(0), data(0), total_capacity(0) , _data_indices(0) , _free_dequeue(0) , _free_enqueue(0) {
	}


	ChannelArray::~ChannelArray() {
		if (_data_indices != 0) {
			//gDefaultMemory->deallocate(_data_indices);
			DEALLOC(_data_indices);
			//delete _data_indices;
		}
		if (data != 0) {
			//gDefaultMemory->deallocate(data);
			DEALLOC(data);
			//delete[] data;
		}
	}

	void ChannelArray::init(int* sizes, int num) {
		assert(num < MAX_BLOCKS);
		for (int i = 0; i < num; ++i) {
			_sizes[i] = sizes[i];
		}
		_num_blocks = num;
	}

	ID ChannelArray::add() {
		if (size + 1 > capacity) {
			resize(size * 2 + 8);
		}
		Index &in = _data_indices[_free_dequeue];
		_free_dequeue = in.next;
		in.index = size++;
		return in.id;
	}

	bool ChannelArray::resize(int new_size) {
		if (new_size > capacity) {
			int total = 0;
			for (int i = 0; i < _num_blocks; ++i) {
				total += _sizes[i];
			}
			if (_data_indices == 0) {
				//_data_indices = (Index*)gDefaultMemory->allocate(new_size * sizeof(Index));
				_data_indices = (Index*)ALLOC(new_size * sizeof(Index));
				//_data_indices = new Index[new_size];// *sizeof(Index));
				for (unsigned short i = 0; i < new_size; ++i) {
					_data_indices[i].id = i;
					_data_indices[i].next = i + 1;
				}
				_free_dequeue = 0;
				_free_enqueue = new_size - 1;
			}
			else {
				//Index* tmp = (Index*)gDefaultMemory->allocate(new_size * sizeof(Index));
				Index* tmp = (Index*)ALLOC(new_size * sizeof(Index));
				//Index* tmp = new Index[new_size];// *)gDefaultMemory->allocate(new_size * sizeof(Index));
				memcpy(tmp, _data_indices, size * sizeof(Index));
				for (unsigned short i = size; i < new_size; ++i) {
					tmp[i].id = i;
					tmp[i].next = i + 1;
				}
				//gDefaultMemory->deallocate(_data_indices);
				DEALLOC(_data_indices);
				//delete[] _data_indices;
				_data_indices = tmp;
				_free_enqueue = new_size - 1;
			}
			int sz = new_size * total;
			//char* t = (char*)gDefaultMemory->allocate(sz);
			char* t = (char*)ALLOC(sz);
			//char* t = new char[sz];
			if (data != 0) {
				int offset = 0;
				int old_offset = 0;
				for (int i = 0; i < _num_blocks; ++i) {
					memcpy(t + offset, data + old_offset, size * _sizes[i]);
					offset += new_size * _sizes[i];
					old_offset += capacity * _sizes[i];
				}
				//gDefaultMemory->deallocate(data);
				DEALLOC(data);
				//delete[] data;
			}
			capacity = new_size;
			_indices[0] = 0;
			for (int i = 1; i < _num_blocks; ++i) {
				_indices[i] = _indices[i - 1] + _sizes[i] * capacity;
			}
			data = t;
			return true;
		}
		return false;
	}

	void* ChannelArray::get_ptr(int index) {
		return data + _indices[index];
	}

	void ChannelArray::remove(ID id) {
		Index &in = _data_indices[id & INDEX_MASK];
		assert(in.index != USHRT_MAX);
		int current = in.index;
		int next = (size - 1);
		for (int i = 0; i < _num_blocks; ++i) {

			memcpy(data + current, data + next, _sizes[i]);
			current += capacity * _sizes[i];
			next += capacity * _sizes[i];
		}
		--size;
		_data_indices[in.id & INDEX_MASK].index = in.index;
		in.index = USHRT_MAX;
		_data_indices[_free_enqueue].next = id & INDEX_MASK;
		_free_enqueue = id & INDEX_MASK;
	}

}
