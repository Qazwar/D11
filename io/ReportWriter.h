#pragma once
#include <stdint.h>
#include <stdio.h>
#include <Vector.h>
#include "..\renderer\render_types.h"

namespace ds {

	class ReportWriter {

	public:
		ReportWriter(const char* fileName);
		~ReportWriter();
		void addHeader(const char* header) const;
		void startBox(const char* header) const;
		void endBox() const;
		void addSubHeader(const char* header) const;
		void startTable(const char** columnNames, int num) const;
		void startRow() const;
		void addCell(int v) const;
		void addCell(uint32_t v) const;
		void addCell(const v2& v) const;
		void addCell(const Rect& v) const;
		void addCell(bool v) const;
		void addCell(float v) const;
		void addCell(const char* v) const;
		void addCell(int ident, const char* v) const;
		void endRow() const;
		void endTable() const;
	private:
		FILE* _file;
		bool _open;
	};

}