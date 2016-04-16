#include "CubicBezierPath.h"
#include "..\utils\Log.h"
#include <assert.h>

namespace ds {

	CubicBezierPath::CubicBezierPath() {}

	CubicBezierPath::~CubicBezierPath()	{}

	void CubicBezierPath::create(const v2& p0,const v2& p1,const v2& p2,const v2& p3) {
		m_Elements.clear();
		BezierCurve be;
		be.p0 = p0;
		be.p1 = p1;
		be.p2 = p2;
		be.p3 = p3;
		m_Elements.push_back(be);
	}

	void CubicBezierPath::add(const v2& p1,const v2& p2,const v2& p3) {
		assert(m_Elements.size() > 0);
		BezierCurve be;
		be.p0 = m_Elements[m_Elements.size()-1].p3;
		be.p1 = p1;
		be.p2 = p2;
		be.p3 = p3;
		m_Elements.push_back(be);
	}

	void CubicBezierPath::build() {
		assert(m_Elements.size() > 0);
		m_ArcLength[0] = 0.0f;
		
		v2 p;
		v2 previous;
		m_TotalLength = 0.0f;
		for (int i = 0; i <= MAX_CBP_STEPS; ++i) {
			float t = (float)i / static_cast<float>(MAX_CBP_STEPS);
			get(t, &p);
			//dot.x = _bezier_point(t, start.x, c->pt[0].x,
				//c->pt[1].x, c->pt[2].x);
			//dot.y = _bezier_point(t, start.y, c->pt[0].y,
				//c->pt[1].y, c->pt[2].y);
			if (i > 0) {
				float x_diff = p.x - previous.x;
				double y_diff = p.y - previous.y;
				m_TotalLength += sqrt(x_diff * x_diff + y_diff * y_diff);
				m_ArcLength[i] = m_TotalLength;
			}
			previous = p;
		}
		for ( int i = 0; i < m_Elements.size(); ++i ) {
			m_Elements[i].build();
		}
	}

	void CubicBezierPath::approx(float u, v2* p) {
		assert(u >= 0.0f && u <= 1.0f);
		float t = find(u);
		get(t, p);
	}

	float CubicBezierPath::find(float u) {
		float targetLength = u * m_TotalLength;// [32];
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

	void CubicBezierPath::tanget(float u, v2* tangent) {
		assert(u >= 0.0f && u <= 1.0f);
		float t = find(u);
		float ds = 1.0f / m_Elements.size();
		int idx = t * m_Elements.size();
		if (idx == m_Elements.size()) {
			*tangent = m_Elements[m_Elements.size() - 1].p3;
		}
		else {
			BezierCurve& curve = m_Elements[idx];
			float nn = t / ds - idx;
			curve.getTangent(nn, tangent);
		}
	}

	void CubicBezierPath::get(float t,v2* p) {
		float ds = 1.0f / m_Elements.size();
		int idx = t * m_Elements.size();
		if ( idx == m_Elements.size() ) {
			*p = m_Elements[m_Elements.size()-1].p3;
		}
		else {
			BezierCurve& curve = m_Elements[idx];			
			float nn = t / ds - idx;	
			curve.get(nn,p);
		}
	}
	/*
	void CubicBezierPath::load(BinaryLoader* loader) {
		m_Elements.clear();
		v2 p0,p1,p2,p3;
		while ( loader->openChunk() == 0 ) {		
			if ( loader->getChunkID() == 1 ) {						
				loader->read(&p0);
				loader->read(&p1);
				loader->read(&p2);
				loader->read(&p3);
				create(p0,p1,p2,p3);
			}
			else if ( loader->getChunkID() == 2 ) {
				loader->read(&p1);
				loader->read(&p2);
				loader->read(&p3);
				add(p1,p2,p3);
			}
			loader->closeChunk();
		}		
		build();
	}
	*/
}