#pragma once
#include <Vector.h>
#include "..\lib\collection_types.h"
#include "BezierCurve.h"

namespace ds {

	const int MAX_CBP_STEPS = 64;

	class CubicBezierPath {

	typedef Array<BezierCurve> BezierElements;

	public:
		CubicBezierPath();
		~CubicBezierPath();
		void create(const v2& p0,const v2& p1,const v2& p2,const v2& p3);
		void add(const v2& p1,const v2& p2,const v2& p3);	
		void build();
		void get(float t,v2* p);
		void approx(float u, v2* p);
		void tanget(float u, v2* tangent);
		const int size() const {
			return m_Elements.size();
		}
		const BezierCurve& getElement(int idx) const {
			return m_Elements[idx];
		}
	private:
		float find(float u);
		BezierElements m_Elements;
		float m_TotalLength;
		float m_ArcLength[MAX_CBP_STEPS+1];
	};

}