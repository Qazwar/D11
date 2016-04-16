#include "AdditionalData.h"
#include "..\utils\Log.h"

namespace ds {

	// -----------------------------------------------------
	// attach
	// -----------------------------------------------------
	void* AdditionalData::attach(SID sid, int size,int identifier) {
		int idx = find_free_header(size);
		if (idx == -1) {
			AdditionalDataHeader header;
			header.sid = sid;
			header.size = size;
			header.index = data.size;
			header.used = true;
			header.identifier = identifier;
			//LOG << "==> attached sid: " << sid << " size: " << size << " identifier: " << identifier << " index: " << header.index;
			headers.push_back(header);
			return data.alloc(size);
		}
		else {
			AdditionalDataHeader& header = headers[idx];
			header.used = true;
			header.sid = sid;
			header.identifier = identifier;
			//LOG << "==> reusing sid: " << sid << " size: " << size << " identifier: " << identifier << " index: " << header.index;
			return data.data + header.index;
		}
	}

	// -----------------------------------------------------
	// remove
	// -----------------------------------------------------
	void AdditionalData::remove(SID sid) {
		int idx = find_header(sid);
		//LOG << "==> removing sid: " << sid << " header index: " << idx;
		if (idx != -1) {
			AdditionalDataHeader& header = headers[idx];
			if (header.used) {
				header.used = false;
			}
		}
	}

	// -----------------------------------------------------
	// get
	// -----------------------------------------------------
	void* AdditionalData::get(SID sid) {
		int idx = find_header(sid);
		if (idx != -1) {
			const AdditionalDataHeader& header = headers[idx];
			if (header.used) {
				return data.data + header.index;
			}
		}
		//LOG << "No data found for: " << sid;
		debug();
		return 0;
	}

	// -----------------------------------------------------
	// contains
	// -----------------------------------------------------
	bool AdditionalData::contains(SID sid) {
		int idx = find_header(sid);
		if (idx != -1) {
			const AdditionalDataHeader& header = headers[idx];
			return header.used;
		}
		return false;
	}

	// -----------------------------------------------------
	// find free header
	// -----------------------------------------------------
	int AdditionalData::find_free_header(int size) {
		for (uint32_t i = 0; i < headers.size(); ++i) {
			const AdditionalDataHeader& h = headers[i];
			if (h.size == size && !h.used) {
				return i;
			}
		}
		return -1;
	}

	// -----------------------------------------------------
	// find header
	// -----------------------------------------------------
	int AdditionalData::find_header(SID sid) {
		for (uint32_t i = 0; i < headers.size(); ++i) {
			const AdditionalDataHeader& h = headers[i];
			if (h.sid == sid && h.used ) {
				return i;
			}
		}
		return -1;
	}

	// -----------------------------------------------------
	// debug
	// -----------------------------------------------------
	void AdditionalData::debug() {
		for (uint32_t i = 0; i < headers.size(); ++i) {
			const AdditionalDataHeader& h = headers[i];
			LOG << h.index << " sid: " << h.sid << " size: " << h.size << " used: " << h.used << " identifier: " << h.identifier;
		}
	}

	// -----------------------------------------------------
	// save report
	// -----------------------------------------------------
	void AdditionalData::save(const ReportWriter& writer) {
		writer.startBox("Additional Data");
		const char* HEADERS[] = { "Index", "ID", "Size", "Used", "Identifier"};
		writer.startTable(HEADERS, 5);
		for (uint32_t i = 0; i < headers.size(); ++i) {
			const AdditionalDataHeader& h = headers[i];
			writer.startRow();
			writer.addCell(h.index);
			writer.addCell(h.sid);
			writer.addCell(h.size);
			writer.addCell(h.used);
			writer.addCell(h.identifier);
			writer.endRow();
		}
		writer.endTable();
		writer.endBox();
	}

}