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

	struct Mesh {

		Array<PNTCVertex> vertices;

		void add(const v3& position, const v3& normal, const v2& uv) {
			vertices.push_back(PNTCVertex(position, normal, uv, Color::WHITE));
		}

		void add(const v3& position, const v3& normal, const v2& uv,const Color& color) {
			vertices.push_back(PNTCVertex(position, normal, uv, color));
		}


	};

	struct PNTCConstantBuffer {
		mat4 viewProjectionMatrix;
		mat4 worldMatrix;
		v3 cameraPos;
		v3 lightPos;
		float tmp;
		float more;
	};

	class MeshBuffer {

	public:
		MeshBuffer(const MeshBufferDescriptor& descriptor);
		~MeshBuffer();
		void drawImmediate(Mesh* mesh, const v3& position, const v3& scale = v3(1,1,1), const v3& rotation = v3(0,0,0));
		void add(const v3& position, const v3& normal, const v2& uv, const Color& color = Color(255, 255, 255, 255));
		void add(const PNTCVertex& v);
		void add(Mesh* mesh, const v3& position, const v3& scale = v3(1, 1, 1), const v3& rotation = v3(0, 0, 0));
		//void reset();
		void begin();
		void end();
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
		void flush();
		uint32_t _size;
		MeshBufferDescriptor _descriptor;
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

