#pragma once
#include <stdint.h>
#include "render_types.h"

class SpriteBuffer {

	struct Sprite {
		v2 position;
		Texture texture;
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
	int _maxSprites;
	bool _started;
	// resources
	int _indexBuffer;
	int _layoutIndex;
	int _shaderIndex;
};
