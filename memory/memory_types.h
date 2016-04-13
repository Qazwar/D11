#pragma once
#include <stdint.h>

namespace ds {

const uint32_t HEADER_PAD_VALUE = 0xffffffffu;


class Allocator {

public:

	template <class T, class P1> T *make_new(const P1 &p1) {return new (allocate(sizeof(T))) T(p1);}

	template <class T> T *make_new() {return new (allocate(sizeof(T))) T();}

	template <class T> void make_delete(T *p) {
		if (p) {
			p->~T();
			deallocate(p);
		}
	}
	/// Default alignment for memory allocations.
	static const uint32_t DEFAULT_ALIGN = 4;

	Allocator() {}
	virtual ~Allocator() {}

	/// Allocates the specified amount of memory aligned to the specified alignment.
	virtual void *allocate(uint32_t size, uint32_t align = DEFAULT_ALIGN) = 0;

	/// Frees an allocation previously made with allocate().
	virtual void deallocate(void *p) = 0;

	static const uint32_t SIZE_NOT_TRACKED = 0xffffffffu;

	/// Returns the amount of usable memory allocated at p. p must be a pointer
	/// returned by allocate() that has not yet been deallocated. The value returned
	/// will be at least the size specified to allocate(), but it can be bigger.
	/// (The allocator may round up the allocation to fit into a set of predefined
	/// slot sizes.)
	///
	/// Not all allocators support tracking the size of individual allocations.
	/// An allocator that doesn't support it will return SIZE_NOT_TRACKED.
	virtual uint32_t allocated_size(void *p) = 0;

	/// Returns the total amount of memory allocated by this allocator. Note that the 
	/// size returned can be bigger than the size of all individual allocations made,
	/// because the allocator may keep additional structures.
	///
	/// If the allocator doesn't track memory, this function returns SIZE_NOT_TRACKED.
	virtual uint32_t total_allocated() = 0;

	virtual void debug() {}

private:
	/// Allocators cannot be copied.
	Allocator(const Allocator& other);
	Allocator& operator=(const Allocator& other);
};

namespace memory {
	inline void *align_forward(void *p, uint32_t align);
}

inline void *memory::align_forward(void *p, uint32_t align) {
	uintptr_t pi = uintptr_t(p);
	const uint32_t mod = pi % align;
	if (mod)
		pi += (align - mod);
	return (void *)pi;
}

}
/// Creates a new object of type T using the allocator a to allocate the memory.
#define DS_NEW(a, T, ...)		(new ((a).allocate(sizeof(T), sizeof(T))) T(__VA_ARGS__))

/// Frees an object allocated with MAKE_NEW.
#define DS_DELETE(a, T, p)	do {if (p) {(p)->~T(); a.deallocate(p);}} while (0)

