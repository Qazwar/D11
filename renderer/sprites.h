#pragma once
#include <stdint.h>
#include "render_types.h"

class SpriteBuffer {

	struct Sprite {
		v2 position;
		Texture texture;
	};

	struct SpriteVertex {
		v3 position;
		v2 texture;
		SpriteVertex() : position(0, 0, 0) {}
		SpriteVertex(const v3& p, const v2& t) : position(p), texture(t) {}
		SpriteVertex(const v2& p, const v2& t) : position(p,1.0f), texture(t) {}
	};

public:
	SpriteBuffer(int maxSprites);
	~SpriteBuffer();
	void draw(const v2& position, const Texture& texture);
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
