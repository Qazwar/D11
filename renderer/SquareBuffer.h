#pragma once
#include <stdint.h>
#include "render_types.h"
#include "core\graphics\Color.h"
#include "graphics.h"
#include "..\resources\ResourceDescriptors.h"
#include "..\sprites\Sprite.h"
#include "core\lib\collection_types.h"
#include "VertexTypes.h"

namespace ds {

	class SquareBuffer {

	public:
		SquareBuffer(const SquareBufferDescriptor& descriptor);
		~SquareBuffer();
		void drawLine(const v3& start, const v3& end, float thickness, const Texture& t, const Color& color = Color(255, 255, 255, 255));
		void drawLine(const v3& start, const v3& end, const v3& offset, const Texture& t, const Color& color = Color(255, 255, 255, 255));
		void draw(v3* positions, const Texture& t, const Color& color = Color(255, 255, 255, 255));
		void begin();
		void end();
	private:
		void flush();
		int _index;
		SquareBufferDescriptor _descriptor;
		QuadVertex* _vertices;
		int _maxSprites;
		bool _started;
		SpriteBufferCB _constantBuffer;
		RID _currentMtrl;
	};

}

