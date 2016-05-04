#pragma once
#include <stdint.h>
#include "render_types.h"
#include "..\utils\Color.h"
#include "graphics.h"
#include "..\resources\ResourceDescriptors.h"
#include "..\sprites\Sprite.h"
#include "..\lib\collection_types.h"

namespace ds {

	struct QuadVertex {
		v3 position;
		v2 texture;
		Color color;

		QuadVertex() : position(0, 0, 0) {}
		QuadVertex(const v3& p, const v2& t, const Color& c) : position(p), texture(t), color(c) {}
		QuadVertex(const v2& p, const v2& t, const Color& c) : position(p, 1.0f), texture(t), color(c) {}
	};

	class Mesh {

	public:
		Mesh(const MeshDescriptor& descriptor);
		~Mesh();
		void add(const v3& position, const v2& uv, const Color& color = Color(255, 255, 255, 255));
		void draw();
		void rotateX(float angle);
		void rotateY(float angle);
		void rotateZ(float angle);
		void translate(const v3& position);
		void scale(const v3& scale);
	private:
		MeshDescriptor _descriptor;
		v3 _position;
		v3 _scale;
		v3 _rotation;
		Array<QuadVertex> _vertices;
		mat4 _world;
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

