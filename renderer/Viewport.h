#pragma once
#include <Vector.h>

namespace ds {

class Viewport {

public:
	Viewport();
	Viewport(int screenWidth, int screenHeight, int worldWidth, int worldHeight);
	~Viewport(void);
	void setDimension(int width,int height);

	void setDimension(int screenWidth, int screenHeight, int worldWidth, int worldHeight);

	void setPosition(int x,int y);
	void setPosition(const v2& pos);
	const v2& getPosition() const {
		return m_Position;
	}	
	float getPositionX() {
		return m_Position.x;
	}
	float getPositionY() {
		return m_Position.y;
	}
	v2 convertToWorld(const v2& screenPos) const {
		v2 wp;
		float ddx = m_WorldSize.x - m_ScreenSize.x;
		float ddy = m_WorldSize.y - m_ScreenSize.y;
		float dx = m_Position.x - m_ScreenSize.x / 2.0f;
		if (dx < 0.0f) {
			dx = 0.0f;
		}
		if (dx > ddx) {
			dx = ddx;
		}
		wp.x = screenPos.x + dx;

		float dy = m_Position.y - m_ScreenSize.y / 2.0f;
		if (dy < 0.0f) {
			dy = 0.0f;
		}
		if (dy > ddy) {
			dy = ddy;
		}
		wp.y = screenPos.y + dy;
		return wp;
	}
	v2 convertToScreen(const v2& worldPos) const {
		v2 p = worldPos;
		p.x /= m_ViewScaleX;
		p.y /= m_ViewScaleY;
		return p;
	}
	const Vector2f& getWorldSize() const {
		return m_WorldSize;
	}
	const v2& getScreenSize() const {
		return m_ScreenSize;
	}
	const bool isInside(const v2& pos,const v2& extent);
	void setID(int id) {
		_id = id;
	}
	int getID() const {
		return _id;
	}
private:
	v2 m_Position;
	v2 m_ViewPos;
	v2 m_Min;
	v2 m_Max;
	v2 m_ScreenSize;
	v2 m_WorldSize;
	float m_ScaleX;
	float m_ScaleY;
	float m_ViewScaleX;
	float m_ViewScaleY;
	int _id;
};

};

