#pragma once
#include <Vector.h>
#include "..\lib\collection_types.h"
#include "CubicBezierPath.h"

namespace ds {

	struct LineSegment {

		Vector2f start;
		Vector2f end;
	};

	typedef Array<LineSegment> LineElements;

	class StraightPath {

	public:
		StraightPath();
		virtual ~StraightPath();
		virtual void create(const Vector2f& start, const Vector2f& end);
		virtual void add(const Vector2f& end);
		void build();
		void get(float t, Vector2f* p);
		void approx(float u, Vector2f* p);
		void tanget(float u, Vector2f* tangent);
		const int size() const {
			return m_Elements.size();
		}
	protected:
		float find(float u);
		LineElements m_Elements;
		float m_TotalLength;
		float m_ArcLength[MAX_CBP_STEPS + 1];
	};

	class GridPath : public StraightPath {

	public:
		GridPath(float gridSize,const Vector2f& offset);
		~GridPath();
		virtual void create(const Vector2f& start, const Vector2f& end);
		virtual void add(const Vector2f& end);
	private:
		float m_GridSize;
		float m_HalfGridSize;
		Vector2f m_Offset;
	};
	
	
}