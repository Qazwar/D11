#pragma once
#include "memory_types.h"
#include "..\io\ReportWriter.h"
#include <vector>
//#include "CallStackTracer.h"

#ifdef DEBUG
#define ALLOC(size) ds::gDefaultMemory->allocate(size,__FILE__,__LINE__);
#else
#define ALLOC(size) ds::gDefaultMemory->allocate(size);
#endif
#define ALLOCA(size,align) ds::gDefaultMemory->allocate(size,align);
#define DEALLOC(ptr) do { ds::gDefaultMemory->deallocate(ptr);} while(0)

#define MAKE_NEW(T, ...)(new (gDefaultMemory->allocate(sizeof(T), __FILE__,__LINE__)) T(__VA_ARGS__))
#define MAKE_DELETE(T, p)do {if (p) {(p)->~T(); gDefaultMemory->deallocate(p);}} while (0)



namespace ds {

	struct MemoryInfo {

		int headersTotal;
		int headersUsed;
		int capacity;
		uint32_t allocated;
		float percentage;

	};
// -------------------------------------------------------
//  Header
// -------------------------------------------------------
struct Header {
	uint32_t size;
	bool used;
	uint32_t index;
	uint32_t originalSize;
	int allocInfo;

	Header() : size(0), used(false) , index(0) , originalSize(0) , allocInfo(-1) {}
};

struct AllocInfo {
	//int nameIndex;
	//char name[48];
	//int line;
	int size;
	bool reuse;
	bool open;
	int index;
};

// -------------------------------------------------------
// MallocAllocator
// -------------------------------------------------------
class CallStackTracer;

class DefaultAllocator : public Allocator {

typedef std::vector<AllocInfo> AllocInfos;

public:
	DefaultAllocator(uint32_t size = 256 * 1024 * 1024);
	~DefaultAllocator();
	void* allocate(uint32_t size, uint32_t align = 0 );
	void* allocate(uint32_t size, const char* file,int line);
	void deallocate(void *p);
	uint32_t allocated_size( void *p );
	uint32_t total_allocated();
	MemoryInfo get_info();
	void debug();
	void save(const ReportWriter& writer);
	void printOpenAllocations();
private:
	Header* _headers;
	int _num;
	int _header_capacity;
	char* _buffer;
	uint32_t _capacity;
	AllocInfos _infos;
	CallStackTracer* _tracer;
};

extern DefaultAllocator* gDefaultMemory;

}
