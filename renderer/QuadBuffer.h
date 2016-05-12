#pragma once
#include <stdint.h>
#include "render_types.h"
#include "..\utils\Color.h"
#include "graphics.h"
#include "..\resources\ResourceDescriptors.h"
#include "..\sprites\Sprite.h"
#include "..\lib\collection_types.h"
#include "VertexTypes.h"

namespace ds {

	struct PNTCConstantBuffer {
		mat4 viewProjectionMatrix;
		mat4 worldMatrix;
		v3 cameraPos;
		v3 lightPos;
		float tmp;
		float more;
	};

	class QuadBuffer {

	public:
		QuadBuffer(const QuadBufferDescriptor& descriptor);
		~QuadBuffer();
		void draw(const v3& position, const v2& uv, const Color& color = Color(255, 255, 255, 255));
		void begin();
		void end();
	private:
		void flush();
		int _index;
		QuadBufferDescriptor _descriptor;
		QuadVertex* _vertices;
		int _maxSprites;
		bool _started;
	};

}

