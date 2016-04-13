#include "DefaultAllocator.h"
#include "..\utils\Log.h"
#include "..\utils\StringUtils.h"
//#include "CallStackTracer.h"

namespace ds {

DefaultAllocator* gDefaultMemory;
	
DefaultAllocator::DefaultAllocator(uint32_t size) : _capacity(size) , _tracer(0) {
	_buffer = (char*)malloc(sizeof(char) * size);
	LOG << "allocated: " << _capacity;
	_headers = (Header*)malloc(sizeof(Header) * 32);
	_num = 0;
	_header_capacity = 32;
	
}


DefaultAllocator::~DefaultAllocator() {
	//assert(total_allocated() == 0);
	if (total_allocated() > 0) {
		LOG << "--------------- Memory leaks ----------------------------";
		for (int i = 0; i < _num; ++i) {
			const Header& h = _headers[i];
			if (h.used) {
				LOG << i << " size: " << h.size << " index: " << h.index << " used: " << h.used;
			}
		}
	}
	free(_buffer);
	free(_headers);
	delete _tracer;
}

void* DefaultAllocator::allocate(uint32_t size, const char* file, int line) {
	//if (_tracer == 0) {
		//_tracer = new CallStackTracer;
	//}
	char buffer[128];
	string::file_name(file, buffer);
	//LOG << "File: " << buffer << " line: " << line << " size: " << size;
	AllocInfo info;
	//info.line = line;
	info.size = size;
	info.reuse = false;
	info.open = true;
	info.index = -1;// _tracer->create();
	//sprintf_s(info.name, 48, buffer);
	uint32_t s = size;
	int idx = -1;
	for (int i = 0; i < _num; ++i) {
		const Header& h = _headers[i];
		if (!h.used && h.size >= s && idx == -1) {
			idx = i;
		}
	}
	if (idx == -1) {
		info.reuse = false;
		Header h;
		h.used = true;
		h.size = s;
		h.originalSize = s;
		h.index = 0;
		if (_num > 0) {
			const Header& last = _headers[_num - 1];
			h.index = last.index + last.size;
		}
		if (_num + 1 > _header_capacity) {
			int sz = _header_capacity * 2 + 16;
			Header* tmp = (Header*)malloc(sizeof(Header) * sz);
			memcpy(tmp, _headers, sizeof(Header) * _num);
			_header_capacity = sz;
			free(_headers);
			_headers = tmp;
		}
		h.allocInfo = _infos.size();
		_headers[_num++] = h;
		assert(h.index + size < _capacity);
		void* p = _buffer + h.index;

		_infos.push_back(info);
		return p;
	}
	else {
		info.reuse = true;
		Header& h = _headers[idx];
		h.used = true;
		void* p = _buffer + h.index;
		h.allocInfo = _infos.size();
		_infos.push_back(info);
		return p;
	}
}

void* DefaultAllocator::allocate(uint32_t size, uint32_t align) {
	uint32_t s = size + align;
	int idx = -1;
	for (int i = 0; i < _num; ++i) {
		const Header& h = _headers[i];
		if (!h.used && h.size >= s && idx == -1) {
			idx = i;
		}
	}
	if (idx == -1) {
		Header h;
		h.used = true;
		h.size = s;
		h.originalSize = s;
		h.index = 0;
		if ( _num > 0 ) {
			const Header& last = _headers[_num - 1];
			h.index = last.index + last.size;
		}
		if (_num + 1 > _header_capacity) {
			int sz = _header_capacity * 2 + 16;
			Header* tmp = (Header*)malloc(sizeof(Header) * sz);
			memcpy(tmp, _headers, sizeof(Header) * _num);
			_header_capacity = sz;
			free(_headers);
			_headers = tmp;
		}
		_headers[_num++] = h;
		assert(h.index + size < _capacity);
		void* p = _buffer + h.index;
		return p;
	}
	else {
		Header& h = _headers[idx];
		h.used = true;
		void* p = _buffer + h.index;
		return p;
	}
}

void DefaultAllocator::deallocate(void *p) {
	if (!p)
		return;
	int d = (int)((char*)p - _buffer);
	for (int i = 0; i < _num; ++i) {
		Header& h = _headers[i];
		if (h.index == d) {
			h.used = false;
			if (h.allocInfo != -1) {
				_infos[h.allocInfo].open = false;
			}
		}
	}
}

uint32_t DefaultAllocator::allocated_size(void *p) {
	int d = (int)((char*)p - _buffer);
	for (int i = 0; i < _num; ++i) {
		const Header& h = _headers[i];
		if (h.index == d && h.used) {
			return h.size;
		}
	}
	return 0;
}

uint32_t DefaultAllocator::total_allocated() {
	uint32_t total = 0;
	for (int i = 0; i < _num; ++i) {
		const Header& h = _headers[i];
		if (h.used) {
			total += h.size;
		}
	}
	return total;
}

MemoryInfo DefaultAllocator::get_info() {
	MemoryInfo info;
	info.allocated = 0;
	info.headersTotal = _num;
	info.headersUsed = 0;
	info.capacity = _capacity;
	for (int i = 0; i < _num; ++i) {
		const Header& h = _headers[i];
		if (h.used) {
			info.allocated += h.size;
			++info.headersUsed;
		}
	}
	info.percentage = static_cast<float>(info.allocated) / static_cast<float>(_capacity)* 100.0f;
	return info;
}

void DefaultAllocator::debug() {
	MemoryInfo info = get_info();
	LOG << "Allocated memory: " << info.allocated << " in " << info.headersUsed << " blocks";
	//for (int i = 0; i < _num; ++i) {
		//const Header& h = _headers[i];
		//LOG << i << " size: " << h.size << " (" << h.originalSize << ") index : " << h.index << " used : " << h.used;
	//}
}



void DefaultAllocator::save(const ReportWriter& writer) {
	writer.startBox("Memory Dump");
	MemoryInfo info = get_info();
	char nn[64];
	string::format_number(info.allocated, nn);
	const char* OVERVIEW_HEADERS[] = { "Type", "Value" };
	writer.startTable(OVERVIEW_HEADERS, 2);
	writer.startRow();
	writer.addCell("Total headers");
	writer.addCell(info.headersTotal);
	writer.endRow();
	writer.startRow();
	writer.addCell("Used headers");
	writer.addCell(info.headersUsed);
	writer.endRow();
	writer.startRow();
	writer.addCell("Capacity");
	string::format_number(info.capacity, nn);
	writer.addCell(nn);
	writer.endRow();
	writer.startRow();
	writer.addCell("Allocated");
	string::format_number(info.allocated, nn);
	writer.addCell(nn);
	writer.endRow();
	writer.startRow();
	writer.addCell("Used %");
	writer.addCell(info.percentage);
	writer.endRow();

	writer.endTable();
	const char* HEADERS[] = { "Index", "Size", "Original Size", "Block Index", "In use" };
	writer.startTable(HEADERS, 5);
	for (int i = 0; i < _num; ++i) {
		const Header& h = _headers[i];
		writer.startRow();
		writer.addCell(i);
		writer.addCell(h.size);
		writer.addCell(h.originalSize);
		writer.addCell(h.index);
		writer.addCell(h.used);
		writer.endRow();
	}
	writer.endTable();
	const char* INFO_HEADERS[] = { "Stack", "Size", "Reuse","Open" };
	writer.startTable(INFO_HEADERS, 4);
	char line[2048];
	for (int i = 0; i < _infos.size(); ++i) {
		const AllocInfo& h = _infos[i];
		writer.startRow();		
		//_tracer->getCalleess(h.index, line, 2048);
		sprintf(line, "HEllo");
		writer.addCell(line);
		writer.addCell(h.size);
		writer.addCell(h.reuse);
		writer.addCell(h.open);
		writer.endRow();
	}
	writer.endTable();
	writer.endBox();
}

void DefaultAllocator::printOpenAllocations() {
	for (int i = 0; i < _infos.size(); ++i) {
		const AllocInfo& h = _infos[i];
		if (h.open) {
			LOG << "Size: " << h.size;
			//_tracer->print(h.index);
		}
	}
}

}