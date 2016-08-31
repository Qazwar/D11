#include "MeshBuffer.h"
#include "..\renderer\graphics.h"
#include "core\math\math_types.h"
#include "core\math\matrix.h"
#include "..\resources\ResourceDescriptors.h"
#include "..\resources\ResourceContainer.h"
#include "core\log\Log.h"
#include "core\profiler\Profiler.h"
#include "core\io\BinaryFile.h"
#include "..\stats\DrawCounter.h"

namespace ds {

	// ------------------------------------------------------
	// Mesh - load
	// ------------------------------------------------------
	void Mesh::load(const char* fileName, const v3& offset) {
		char buffer[256];
		sprintf_s(buffer, 256, "content\\meshes\\%s.mesh", fileName);
		FILE* f = fopen(buffer, "rb");
		if (f) {
			int size = -1;
			fread(&size, sizeof(uint32_t), 1, f);
			for (int i = 0; i < size; ++i) {
				v3 p;
				for (int k = 0; k < 3; ++k) {
					fread(&p.data[k], sizeof(float), 1, f);
				}
				p += offset;
				v3 n;
				for (int k = 0; k < 3; ++k) {
					fread(&n.data[k], sizeof(float), 1, f);
				}
				v2 uv;
				for (int k = 0; k < 2; ++k) {
					fread(&uv.data[k], sizeof(float), 1, f);
				}
				Color color;
				fread(&color.r, sizeof(float), 1, f);
				fread(&color.g, sizeof(float), 1, f);
				fread(&color.b, sizeof(float), 1, f);
				fread(&color.a, sizeof(float), 1, f);
				add(p, n, uv, color);
			}
			fclose(f);
			LOG << "mesh '" << fileName << "' loaded - entries: " << vertices.size();
			buildBoundingBox();			
		}
	}

	// ------------------------------------------------------
	// Mesh - save
	// ------------------------------------------------------
	void Mesh::save(const char* fileName) {
		char buffer[256];
		sprintf_s(buffer, 256, "content\\meshes\\%s.mesh", fileName);
		BinaryFile bf;
		if (bf.open(buffer, FileMode::WRITE)) {
			bf.write(vertices.size());
			for (uint32_t i = 0; i < vertices.size(); ++i) {
				const PNTCVertex& v = vertices[i];
				bf.write(v.position);
				bf.write(v.normal);
				bf.write(v.texture);
				bf.write(v.color);
			}
		}
	}
	
	// ------------------------------------------------------
	// Mesh - build bounding AABBox
	// ------------------------------------------------------
	void Mesh::buildBoundingBox() {
		if (vertices.size() > 0) {
			v3 min_p = v3(10000.0f);
			v3 max_p = v3(0.0f);
			v3 e = v3(0, 0, 0);
			for (uint32_t i = 0; i < vertices.size(); ++i) {
				min_p = math::min_val(min_p, vertices[i].position);
				max_p = math::max_val(max_p, vertices[i].position);
			}
			boundingBox.position = (min_p + max_p) * 0.5f;
			boundingBox.extent = (max_p - min_p) * 0.5f;
		}
		LOG << "AABBox - center: " << DBG_V3(boundingBox.position) << " extent: " << DBG_V3(boundingBox.extent);
	}
	
	// ------------------------------------------------------
	// MeshBuffer
	// ------------------------------------------------------
	MeshBuffer::MeshBuffer(const MeshBufferDescriptor& descriptor) : _descriptor(descriptor) {
		_lightPos = v3(0.25f, 0.25f, -0.3f);
		_size = descriptor.size;
		_diffuseColor = Color::WHITE;
		_buffer.more = 1.0f;
		_buffer.tmp = 1.0f;
		_buffer.diffuseColor = _diffuseColor;
		_buffer.lightPos = _lightPos;
		_vertices = new PNTCVertex[_size];
	}

	MeshBuffer::~MeshBuffer() {
		delete[] _vertices;
	}

	// ------------------------------------------------------
	// add vertex
	// ------------------------------------------------------
	void MeshBuffer::add(const v3& position, const v3& normal, const v2& uv, const Color& color) {
		if (_index + 1 > _size) {
			flush();
		}
		//_vertices.push_back(PNTCVertex(position, normal, uv, color));
		_vertices[_index++] = PNTCVertex(position, normal, uv, color);
	}

	// ------------------------------------------------------
	// add vertex
	// ------------------------------------------------------
	void MeshBuffer::add(const PNTCVertex& v) {
		if (_index + 1 >= _size) {
			flush();
		}
		_vertices[_index++] = v;
		//_vertices.push_back(v);
	}

	// ------------------------------------------------------
	// add mesh
	// ------------------------------------------------------
	void MeshBuffer::add(Mesh* mesh, const v3& position, const v3& scale, const v3& rotation, const Color& color) {
		ZoneTracker z("MeshBuffer::addMesh3");
		mat4 w = matrix::mat4Transform(position);
		add(mesh, w, scale, rotation, color);
	}

	void MeshBuffer::add(Mesh* mesh, const mat4& world, const Color& color) {
		ZoneTracker z("MeshBuffer::addMesh4");
		int cnt = mesh->vertices.size() / 4;
		v3 p[4];
		v2 t[4];
		Color c[4];
		for (int i = 0; i < cnt; ++i) {
			for (int j = 0; j < 4; ++j) {
				const PNTCVertex& v = mesh->vertices[i * 4 + j];
				p[j] = world * v.position;
				t[j] = v.texture;
				c[j] = v.color;
			}
			v3 s = p[0];
			v3 end1 = p[1];
			v3 end2 = p[2];
			v3 a = end1 - s;
			v3 b = end2 - s;
			v3 n = normalize(cross(a, b));
			for (int j = 0; j < 4; ++j) {
				add(p[j], n, t[j], c[j] * color);
			}
		}
	}

	// ------------------------------------------------------
	// add mesh
	// ------------------------------------------------------
	void MeshBuffer::add(Mesh* mesh, const mat4& world, const v3& scale, const v3& rotation, const Color& color) {
		ZoneTracker z("MeshBuffer::addMesh2");
		mat4 rotY = matrix::mat4RotationY(rotation.y);
		mat4 rotX = matrix::mat4RotationX(rotation.x);
		mat4 rotZ = matrix::mat4RotationZ(rotation.z);
		mat4 s = matrix::mat4Scale(scale);
		mat4 w = rotZ * rotY * rotX * s * world;
		for (uint32_t i = 0; i < mesh->vertices.size(); ++i) {
			const PNTCVertex& v = mesh->vertices[i];
			v3 p = w * v.position;
			//v3 n = world * v.normal;
			// FIXME: we need to rotate normal!!!
			v3 n = v.normal;
			add(p, n, v.texture, v.color * color);
		}
	}

	// ------------------------------------------------------
	// add mesh
	// ------------------------------------------------------
	void MeshBuffer::add(Mesh* mesh) {
		ZoneTracker z("MeshBuffer::addMesh");
		for (uint32_t i = 0; i < mesh->vertices.size(); ++i) {
			add(mesh->vertices[i]);
		}
	}

	// ------------------------------------------------------
	// add mesh
	// ------------------------------------------------------
	void MeshBuffer::add(PNTCVertex* vertices,int size) {
		ZoneTracker z("MeshBuffer::addMesh5");
		if (_index + size >= _size) {
			flush();
		}
		memcpy(_vertices + _index, vertices, size * sizeof(PNTCVertex));
		_index += size;
		//for (int i = 0; i < mesh->vertices.size(); ++i) {
			//add(mesh->vertices[i]);
		//}
	}

	// ------------------------------------------------------
	// add mesh
	// ------------------------------------------------------
	void MeshBuffer::add(Mesh* mesh, const v3& position, const Color& color, const v3& scale, const v3& rotation) {
		ZoneTracker z("MeshBuffer::addMesh1");
		mat4 rotY = matrix::mat4RotationY(rotation.y);
		mat4 rotX = matrix::mat4RotationX(rotation.x);
		mat4 rotZ = matrix::mat4RotationZ(rotation.z);
		mat4 t = matrix::mat4Transform(position);
		mat4 s = matrix::mat4Scale(scale);
		mat4 world = rotZ * rotY * rotX * s * t;
		for (uint32_t i = 0; i < mesh->vertices.size(); ++i) {
			const PNTCVertex& v = mesh->vertices[i];
			v3 p = world * v.position;
			//v3 n = world * v.normal;
			v3 n = v.normal;
			add(p, n, v.texture, color);
		}
	}

	// ------------------------------------------------------
	// begin
	// ------------------------------------------------------
	void MeshBuffer::begin() {
		//_vertices.clear();
		_index = 0;
	}

	// ------------------------------------------------------
	// end
	// ------------------------------------------------------
	void MeshBuffer::end() {
		//if (_vertices.size() > 0) {
		if (_index > 0) {
			flush();
		}
	}

	// ------------------------------------------------------
	// draw immediately
	// ------------------------------------------------------
	void MeshBuffer::drawImmediate(Mesh* mesh, const v3& position, const v3& scale, const v3& rotation, const Color& color) {
		mat4 t = matrix::mat4Transform(position);
		drawImmediate(mesh, t, scale, rotation, color);
	}

	// ------------------------------------------------------
	// draw immediately
	// ------------------------------------------------------
	void MeshBuffer::drawImmediate(Mesh* mesh, const mat4& world, const v3& scale, const v3& rotation, const Color& color) {		
		flush();
		ZoneTracker("Mesh::drawImmediate");
		mat4 w = matrix::m4identity();
		mat4 rotY = matrix::mat4RotationY(rotation.y);
		mat4 rotX = matrix::mat4RotationX(rotation.x);
		mat4 rotZ = matrix::mat4RotationZ(rotation.z);
		mat4 s = matrix::mat4Scale(scale);
		w = rotZ * rotY * rotX * s * world;
		unsigned int stride = sizeof(PNTCVertex);
		unsigned int offset = 0;

		graphics::setVertexBuffer(_descriptor.vertexBuffer, &stride, &offset);
		graphics::setIndexBuffer(_descriptor.indexBuffer);
		graphics::setMaterial(_descriptor.material);
		Camera* camera = graphics::getCamera();
		ds::mat4 mvp = w * camera->getViewProjectionMatrix();
		_buffer.viewProjectionMatrix = ds::matrix::mat4Transpose(mvp);
		_buffer.worldMatrix = ds::matrix::mat4Transpose(w);
		_buffer.cameraPos = camera->getPosition();
		_buffer.lightPos = _lightPos;
		_buffer.diffuseColor = color;// Color(192, 0, 0, 255);
		graphics::mapData(_descriptor.vertexBuffer, mesh->vertices.data(), mesh->vertices.size() * sizeof(PNTCVertex));
		graphics::updateConstantBuffer(_descriptor.constantBuffer, &_buffer, sizeof(PNTCConstantBuffer));
		graphics::setVertexShaderConstantBuffer(_descriptor.constantBuffer);
		//graphics::setPixelShaderConstantBuffer(_descriptor.constantBuffer);
		graphics::drawIndexed(mesh->vertices.size() / 4 * 6);
		++gDrawCounter->flushes;
		gDrawCounter->vertices += mesh->vertices.size();
	}

	// ------------------------------------------------------
	// flush
	// ------------------------------------------------------
	void MeshBuffer::flush() {
		if (_index > 0) {
			ZoneTracker("Mesh::flush");

			mat4 world = matrix::m4identity();
			unsigned int stride = sizeof(PNTCVertex);
			unsigned int offset = 0;

			graphics::setVertexBuffer(_descriptor.vertexBuffer, &stride, &offset);
			graphics::setIndexBuffer(_descriptor.indexBuffer);
			graphics::setMaterial(_descriptor.material);

			Camera* camera = graphics::getCamera();
			ds::mat4 mvp = world * camera->getViewProjectionMatrix();
			_buffer.viewProjectionMatrix = ds::matrix::mat4Transpose(mvp);
			_buffer.worldMatrix = ds::matrix::mat4Transpose(world);
			_buffer.cameraPos = camera->getPosition();
			_buffer.lightPos = _lightPos;
			_buffer.diffuseColor = _diffuseColor;
			
			graphics::mapData(_descriptor.vertexBuffer, _vertices, _index * sizeof(PNTCVertex));
			
			graphics::updateConstantBuffer(_descriptor.constantBuffer, &_buffer, sizeof(PNTCConstantBuffer));
			graphics::setVertexShaderConstantBuffer(_descriptor.constantBuffer);
			graphics::drawIndexed(_index / 4 * 6);

			++gDrawCounter->flushes;
			gDrawCounter->vertices += _index;
			_index = 0;			
		}
	}

}