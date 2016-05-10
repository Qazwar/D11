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

	class Mesh {

	public:
		Mesh(const MeshDescriptor& descriptor);
		~Mesh();
		void add(const v3& position, const v3& normal, const v2& uv, const Color& color = Color(255, 255, 255, 255));
		void add(const PNTCVertex& v);
		void reset();
		void draw();
		void rotateX(float angle);
		void rotateY(float angle);
		void rotateZ(float angle);
		void translate(const v3& position);
		void scale(const v3& scale);
		v3* getLightPos() {
			return &_lightPos;
		}
	private:
		MeshDescriptor _descriptor;
		v3 _position;
		v3 _scale;
		v3 _rotation;
		v3 _lightPos;
		Array<PNTCVertex> _vertices;
		mat4 _world;
		PNTCConstantBuffer _buffer;
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

