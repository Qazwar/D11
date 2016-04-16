#include "StraightPath.h"
#include "..\utils\Log.h"
#include <assert.h>

namespace ds {
	
	StraightPath::StraightPath() {}

	StraightPath::~StraightPath()	{}

	void StraightPath::create(const Vector2f& start, const Vector2f& end) {
		m_Elements.clear();
		LineSegment ls;
		ls.start = start;
		ls.end = end;
		m_Elements.push_back(ls);
	}

	void StraightPath::add(const Vector2f& end) {
		assert(m_Elements.size() > 0);
		LineSegment ls;
		ls.start = m_Elements[m_Elements.size() - 1].end;
		ls.end = end;
		m_Elements.push_back(ls);
	}

	void StraightPath::build() {
		assert(m_Elements.size() > 0);
		m_ArcLength[0] = 0.0f;
		Vector2f p;
		Vector2f previous;
		m_TotalLength = 0.0f;
		for (int i = 0; i <= MAX_CBP_STEPS; ++i) {
			float t = (float)i / static_cast<float>(MAX_CBP_STEPS);
			get(t, &p);
			if (i > 0) {
				float x_diff = p.x - previous.x;
				double y_diff = p.y - previous.y;
				m_TotalLength += sqrt(x_diff * x_diff + y_diff * y_diff);
				m_ArcLength[i] = m_TotalLength;
			}
			previous = p;
		}
	}

	void StraightPath::approx(float u, Vector2f* p) {
		assert(u >= 0.0f && u <= 1.0f);
		float t = find(u);
		get(t, p);
	}

	float StraightPath::find(float u) {
		if (u == 0.0f) {
			return 0.0f;
		}
		float targetLength = u * m_TotalLength;
		int low = 0;
		int high = MAX_CBP_STEPS;
		int index = 0;
		while (low < high) {
			index = (low + (high - low) / 2);
			if (m_ArcLength[index] < targetLength) {
				low = index + 1;

			}
			else {
				high = index;
			}
		}
		if (m_ArcLength[index] > targetLength) {
			index--;
		}
		float lengthBefore = m_ArcLength[index];
		if (lengthBefore == targetLength) {
			return lengthBefore;

		}
		else {
			return (index + (targetLength - lengthBefore) / (m_ArcLength[index + 1] - lengthBefore)) / static_cast<float>(MAX_CBP_STEPS);
		}
	}

	void StraightPath::tanget(float u, Vector2f* tangent) {
		assert(u >= 0.0f && u <= 1.0f);
		float t = find(u);
		float ds = 1.0f / m_Elements.size();
		int idx = t * m_Elements.size();
		if (idx == m_Elements.size()) {
			*tangent = m_Elements[m_Elements.size() - 1].end - m_Elements[m_Elements.size() - 1].start;
		}
		else {
			LineSegment& curve = m_Elements[idx];
			float nn = t / ds - idx;
			*tangent = lerp(curve.start, curve.end, nn) - lerp(curve.start, curve.end, nn - 1/60.0f);
		}
	}

	void StraightPath::get(float t, Vector2f* p) {
		float ds = 1.0f / m_Elements.size();
		int idx = t * m_Elements.size();
		if (idx == m_Elements.size()) {
			*p = m_Elements[m_Elements.size() - 1].end;
		}
		else {
			LineSegment& curve = m_Elements[idx];
			float nn = t / ds - idx;
			*p = lerp(curve.start, curve.end, nn);
		}
	}
	/*
	void StraightPath::load(BinaryLoader* loader) {
		m_Elements.clear();
		Vector2f p0, p1;
		while (loader->openChunk() == 0) {
			if (loader->getChunkID() == 1) {
				loader->read(&p0);
				loader->read(&p1);
				create(p0, p1);
			}
			else if (loader->getChunkID() == 2) {
				loader->read(&p1);
				add(p1);
			}
			loader->closeChunk();
		}
		build();
	}
	*/
	GridPath::GridPath(float gridSize, const Vector2f& offset) : StraightPath(), m_GridSize(gridSize) , m_Offset(offset) {
		m_HalfGridSize = m_GridSize * 0.5f;
	}

	GridPath::~GridPath() {}

	void GridPath::create(const Vector2f& start, const Vector2f& end) {
		m_Elements.clear();
		Vector2f newStart, newEnd;
		newStart.x = m_Offset.x + m_HalfGridSize + start.x * m_GridSize;
		newStart.y = m_Offset.y + m_HalfGridSize + start.y * m_GridSize;
		newEnd.x = m_Offset.x + m_HalfGridSize + end.x * m_GridSize;
		newEnd.y = m_Offset.y + m_HalfGridSize + end.y * m_GridSize;
		StraightPath::create(newStart, newEnd);
		//m_Elements.push_back(ls);
	}

	void GridPath::add(const Vector2f& end) {
		assert(m_Elements.size() > 0);
		//LineSegment ls;
		Vector2f newStart, newEnd;
		newStart = m_Elements[m_Elements.size() - 1].end;
		newEnd.x = m_Offset.x + m_HalfGridSize + end.x * m_GridSize;
		newEnd.y = m_Offset.y + m_HalfGridSize + end.y * m_GridSize;
		//m_Elements.push_back(ls);
		StraightPath::add(newEnd);
	}
	
}