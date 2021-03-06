#include "DrawCounter.h"

namespace ds {

	DrawCounter* gDrawCounter;

	void DrawCounter::reset() {
		flushes = 0;
		vertices = 0;
		indices = 0;
		sprites = 0;
		spriteFlushes = 0;
		squares = 0;
	}

	void DrawCounter::save(const ReportWriter& writer) {
		writer.startBox("DrawCounter");
		const char* OVERVIEW_HEADERS[] = { "Type", "Count" };
		writer.startTable(OVERVIEW_HEADERS, 2);
		writer.startRow();
		writer.addCell("Vertices");
		writer.addCell(vertices);
		writer.endRow();
		writer.startRow();
		writer.addCell("Flushes");
		writer.addCell(flushes);
		writer.endRow();
		writer.startRow();
		writer.addCell("Sprites");
		writer.addCell(sprites);
		writer.endRow();
		writer.startRow();
		writer.addCell("Sprite flushes");
		writer.addCell(spriteFlushes);
		writer.endRow();
		writer.startRow();
		writer.addCell("Squares");
		writer.addCell(squares);
		writer.endRow();
		writer.endTable();
		writer.endBox();
	}


}