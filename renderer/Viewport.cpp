#include "Viewport.h"
#include <core\log\Log.h>

namespace ds {

	Viewport::Viewport() : m_Position(0, 0) {
		setDimension(1024, 768,1024,768);
	}

	Viewport::Viewport(int screenWidth, int screenHeight, int worldWidth, int worldHeight) {
		setDimension(screenWidth, screenHeight, worldWidth, worldHeight);
	}

	Viewport::~Viewport() {}

	void Viewport::setDimension(int width,int height) {
		LOGC("viewport") << "Setting dimension to " << width << "x" << height;
		setDimension(width, height, width, height);
	}

	void Viewport::setDimension(int screenWidth, int screenHeight, int worldWidth, int worldHeight) {
		m_ScreenSize = v2(screenWidth, screenHeight);
		m_WorldSize = v2(worldWidth, worldHeight);
		m_ScaleX = m_ScreenSize.x / m_WorldSize.x;
		m_ScaleY = m_ScreenSize.y / m_WorldSize.y;
		m_ViewScaleX = m_WorldSize.x / m_ScreenSize.x;
		m_ViewScaleY = m_WorldSize.y / m_ScreenSize.y;
		LOGC("viewport") << "Viewport scale.x : " << m_ScaleX << " scale.y : " << m_ScaleY;
		float deltaX = (m_WorldSize.x - m_ScreenSize.x) * 0.5f;
		float deltaY = (m_WorldSize.y - m_ScreenSize.y) * 0.5f;
		LOGC("viewport") << "delta " << deltaX << " " << deltaY;
		m_Max = v2(m_WorldSize.x * 0.5f + deltaX, m_WorldSize.y * 0.5f + deltaY);
		m_Min = v2(m_WorldSize.x * 0.5f - deltaX,m_WorldSize.y * 0.5f - deltaY);
		LOGC("viewport") << "max: " << m_Max;
		LOGC("viewport") << "min: " << m_Min;
		setPosition(worldWidth / 2, worldHeight / 2);
	}

	void Viewport::setPosition(int x,int y) {
		setPosition(v2(x,y));	
	}

	void Viewport::setPosition(const v2& pos) {
		m_Position = pos;
		clamp(&m_Position, m_Min, m_Max);	
		m_ViewPos.x = m_Position.x * m_ScaleX;
		m_ViewPos.y = m_Position.y * m_ScaleY;
	}

	const bool Viewport::isInside(const v2& pos,const v2& extent) {
		if ( pos.x < extent.x * 0.5f || pos.x > ( m_WorldSize.x - extent.x * 0.5f )) {
			return false;
		}
		if ( pos.y < extent.y * 0.5f || pos.y > ( m_WorldSize.y - extent.y * 0.5f )) {
			return false;
		}
		return true;
	}

};
