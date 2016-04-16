#pragma once
#include "..\math\math_types.h"
#include "Vector.h"
#include "..\utils\Color.h"
#include "..\renderer\render_types.h"
#include "..\Common.h"

namespace ds {

	enum SpriteShapeType {
		SST_NONE,
		SST_BOX,
		SST_CIRCLE
	};

	struct BasicSprite {

		ID id;
		v2 position;
		v2 scale;
		float rotation;
		Texture texture;
		Color color;
		
		BasicSprite() : id(INVALID_ID), position(0, 0), scale(1, 1), rotation(0.0f), color(Color(255, 255, 255, 255)) {}

	};

	typedef unsigned int SID;
	const unsigned int INVALID_SID = UINT_MAX;

	struct Sprite {

		SID id;
		v2 position;
		v2 scale;
		float rotation;
		Texture texture;
		Color color;
		int type;
		int layer;
		v2 previous;
		v2 extent;
		SpriteShapeType shapeType;

		Sprite() : id(0) , position(0,0) , scale(1,1) , rotation(0.0f) , color(Color(255,255,255,255)) , type(0) , layer(0) , previous(0,0) , extent(0,0) , shapeType(SST_NONE) {}

	};
	
}