#pragma once
#include <stdint.h>
#include "render_types.h"
#include "..\utils\Color.h"
#include "graphics.h"
#include "..\resources\ResourceDescriptors.h"
#include "..\sprites\Sprite.h"
#include "..\lib\collection_types.h"
#include "VertexTypes.h"
#include "QuadBuffer.h"
#include "..\physics\physic_types.h"

namespace ds {

	// ------------------------------------------------------
	// Mesh
	// ------------------------------------------------------
	struct Mesh {

		AABBox boundingBox;
		Array<PNTCVertex> vertices;

		void add(const v3& position, const v3& normal, const v2& uv) {
			vertices.push_back(PNTCVertex(position, normal, uv, Color::WHITE));
		}

		void add(const v3& position, const v3& normal, const v2& uv, const Color& color) {
			vertices.push_back(PNTCVertex(position, normal, uv, color));
		}

		void clear() {
			vertices.clear();
		}

		void load(const char* fileName);

		void buildBoundingBox();

	};

	// ------------------------------------------------------
	// MeshBuffer
	// ------------------------------------------------------
	class MeshBuffer {

	public:
		MeshBuffer(const MeshBufferDescriptor& descriptor);
		~MeshBuffer();
		void drawImmediate(Mesh* mesh, const v3& position, const v3& scale = v3(1, 1, 1), const v3& rotation = v3(0, 0, 0), const Color& color = Color(255,255,255,255));
		void drawImmediate(Mesh* mesh, const mat4& world, const v3& scale = v3(1, 1, 1), const v3& rotation = v3(0, 0, 0), const Color& color = Color(255, 255, 255, 255));
		void add(const v3& position, const v3& normal, const v2& uv, const Color& color = Color(255, 255, 255, 255));
		void add(const PNTCVertex& v);
		void add(Mesh* mesh, const v3& position, const v3& scale = v3(1, 1, 1), const v3& rotation = v3(0, 0, 0), const Color& color = Color(255, 255, 255, 255));
		void add(Mesh* mesh, const mat4& world, const v3& scale = v3(1, 1, 1), const v3& rotation = v3(0, 0, 0), const Color& color = Color(255, 255, 255, 255));
		void add(Mesh* mesh, const v3& position, const Color& color, const v3& scale = v3(1, 1, 1), const v3& rotation = v3(0, 0, 0));
		void begin();
		void end();
		void flush();
		void draw();
		void rotateX(float angle);
		void rotateY(float angle);
		void rotateZ(float angle);
		void translate(const v3& position);
		void scale(const v3& scale);
	private:
		uint32_t _size;
		MeshBufferDescriptor _descriptor;
		v3 _lightPos;
		Array<PNTCVertex> _vertices;
		PNTCConstantBuffer _buffer;
		Color _diffuseColor;
	};

}