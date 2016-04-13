#pragma once
#include <limits.h>
#include <assert.h>

// iterator:
// https://gist.github.com/jeetsukumaran/307264

#define INDEX_MASK 0xffff
#define NEW_OBJECT_ID_ADD 0x10000

typedef unsigned int ID;
const unsigned int INVALID_ID = UINT_MAX; 

namespace ds {

struct Index {
	ID id;
	unsigned short index;
	unsigned short next;
};

template<class U,unsigned short MAX_FLOW_OBJECTS = 4096>
struct DataArray {

	unsigned int numObjects;
	Index indices[MAX_FLOW_OBJECTS];
	U objects[MAX_FLOW_OBJECTS];
	unsigned short free_enqueue;
	unsigned short free_dequeue;

	class iterator {
	public:
		typedef iterator self_type;
		typedef U value_type;
		typedef U& reference;
		typedef U* pointer;
		//typedef std::forward_iterator_tag iterator_category;
		typedef int difference_type;
		iterator(pointer ptr) : ptr_(ptr) { }
		self_type operator++() { self_type i = *this; ptr_++; return i; }
		self_type operator++(int junk) { ptr_++; return *this; }
		reference operator*() { return *ptr_; }
		pointer operator->() { return ptr_; }
		bool operator==(const self_type& rhs) { return ptr_ == rhs.ptr_; }
		bool operator!=(const self_type& rhs) { return ptr_ != rhs.ptr_; }
	private:
		pointer ptr_;
	};

	class const_iterator {

	public:
		typedef const_iterator self_type;
		typedef U value_type;
		typedef U& reference;
		typedef U* pointer;
		typedef int difference_type;
		//typedef std::forward_iterator_tag iterator_category;
		const_iterator(pointer ptr) : ptr_(ptr) { }
		self_type operator++() { self_type i = *this; ptr_++; return i; }
		self_type operator++(int junk) { ptr_++; return *this; }
		const reference operator*() { return *ptr_; }
		const pointer operator->() { return ptr_; }
		bool operator==(const self_type& rhs) { return ptr_ == rhs.ptr_; }
		bool operator!=(const self_type& rhs) { return ptr_ != rhs.ptr_; }
	private:
		pointer ptr_;
	};


	DataArray() {
		clear();
	}

	iterator begin() {
		return iterator(objects);
	}

	iterator end() {
		return iterator(objects + numObjects);
	}

	const_iterator begin() const {
		return const_iterator(objects);
	}

	const_iterator end() const {
		return const_iterator(objects + numObjects);
	}

	void clear() {
		numObjects = 0;
		for ( unsigned short i = 0; i < MAX_FLOW_OBJECTS; ++i ) {
			indices[i].id = i;
			indices[i].next = i + 1;
		}
		free_dequeue = 0;
		free_enqueue = MAX_FLOW_OBJECTS - 1;
	}

	const bool contains(ID id) const {
		const Index& in = indices[id & INDEX_MASK];
		return in.id == id && in.index != USHRT_MAX;
	}

	U& get(ID id) {
		assert( id != UINT_MAX );
		unsigned short index = indices[id & INDEX_MASK].index;
		assert(index != USHRT_MAX);
		return objects[index];
	}

	const U& get(ID id) const {
		assert(id != UINT_MAX);
		unsigned short index = indices[id & INDEX_MASK].index;
		assert(index != USHRT_MAX);
		return objects[index];
	}

	ID add() {
		assert(numObjects != MAX_FLOW_OBJECTS);
		Index &in = indices[free_dequeue];
		free_dequeue = in.next;
		//in.id += NEW_OBJECT_ID_ADD;
		in.index = numObjects++;
		U& o = objects[in.index];
		o.id = in.id;
		return o.id;
	}

	iterator remove(ID id) {
		Index &in = indices[id & INDEX_MASK];
		assert(in.index != USHRT_MAX);
		int current = in.index;
		U& o = objects[in.index];
		o = objects[--numObjects];
		indices[o.id & INDEX_MASK].index = in.index;
		in.index = USHRT_MAX;
		indices[free_enqueue].next = id & INDEX_MASK;
		free_enqueue = id & INDEX_MASK;
		if (numObjects == 0) {
			return begin();
		}
		return iterator(objects + current);
	}
};

struct BaseArray {

	Index* indices;
	ID* ids;
	int num;
	int total;
	char* buffer;

	unsigned short free_enqueue;
	unsigned short free_dequeue;

	BaseArray() : num(0), total(0), buffer(0) {
		clear();
	}
	/*
	virtual ~BaseArray() {
		if (buffer != 0) {
			delete[] buffer;
		}
	}
	*/
	void clear() {
		for (unsigned short i = 0; i < total; ++i) {
			indices[i].id = i;
			indices[i].next = i + 1;
		}
		num = 0;
		free_dequeue = 0;
		free_enqueue = total - 1;
	}

	const bool contains(ID id) const {
		Index& in = indices[id];
		return in.index != USHRT_MAX;
	}

	const int getIndex(ID id) const {
		Index &in = indices[id];
		assert(in.index != USHRT_MAX);
		return in.index;
	}

	ID add() {
		Index &in = indices[free_dequeue];
		free_dequeue = in.next;
		in.index = num++;
		ids[in.index] = in.id;		
		return in.id;
	}

	void remove(ID id) {
		Index &in = indices[id & INDEX_MASK];
		assert(in.index != USHRT_MAX);
		ID currentID = ids[num - 1];
		Index& next = indices[currentID & INDEX_MASK];
		ids[in.index] = ids[next.index];
		swap(in.index, next.index);
		--num;
		indices[currentID & INDEX_MASK].index = in.index;
		in.index = USHRT_MAX;
		indices[free_enqueue].next = id & INDEX_MASK;
		free_enqueue = id & INDEX_MASK;
	}

	virtual void swap(int oldIndex, int newIndex) = 0;
};

}