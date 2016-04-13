#pragma once
#include <stdint.h>
#include "render_types.h"
#include "..\utils\Color.h"

class SpriteBuffer {

	struct Sprite {
		v2 position;
		Texture texture;
		Color color;
	};

	struct SpriteVertex {
		v3 position;
		v2 texture;
		Color color;

		SpriteVertex() : position(0, 0, 0) {}
		SpriteVertex(const v3& p, const v2& t, const Color& c) : position(p), texture(t) , color(c) {}
		SpriteVertex(const v2& p, const v2& t, const Color& c) : position(p,1.0f), texture(t) , color(c) {}
	};

public:
	SpriteBuffer(int maxSprites);
	~SpriteBuffer();
	void draw(const v2& position, const Texture& texture, const Color& color = Color(255,255,255,255));
	void begin();
	void end();
private:
	void flush();
	int _index;
	Sprite* _sprites;
	SpriteVertex* _vertices;
	int _maxSprites;
	bool _started;
	// resources
	int _indexBuffer;
	int _vertexBuffer;
	int _layoutIndex;
	int _shaderIndex;
	int _colorMap;
	int _alphaBlendState;
	int _mvpCB;
};
