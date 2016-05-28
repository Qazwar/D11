#include "MeshBuffer.h"
#include "..\renderer\graphics.h"
#include "..\math\math_types.h"
#include "..\math\matrix.h"
#include "..\resources\ResourceDescriptors.h"
#include "..\resources\ResourceContainer.h"
#include "..\utils\Log.h"
#include "..\utils\Profiler.h"

namespace ds {

	// ------------------------------------------------------
	// MeshBuffer
	// ------------------------------------------------------
	MeshBuffer::MeshBuffer(const MeshBufferDescriptor& descriptor) : _descriptor(descriptor) {
		_lightPos = v3(0.2f, 0.2f, -1.0f);
		_size = descriptor.size;
		_diffuseColor = Color::WHITE;
		_buffer.more = 1.0f;
		_buffer.tmp = 1.0f;
		_buffer.diffuseColor = _diffuseColor;
		_buffer.lightPos = _lightPos;
	}

	MeshBuffer::~MeshBuffer() {
	}

	// ------------------------------------------------------
	// add vertex
	// ------------------------------------------------------
	void MeshBuffer::add(const v3& position, const v3& normal, const v2& uv, const Color& color) {
		if (_vertices.size() + 1 > _size) {
			flush();
		}
		_vertices.push_back(PNTCVertex(position, normal, uv, color));
	}

	// ------------------------------------------------------
	// add vertex
	// ------------------------------------------------------
	void MeshBuffer::add(const PNTCVertex& v) {
		if (_vertices.size() + 1 >= _size) {
			flush();
		}
		_vertices.push_back(v);
	}

	// ------------------------------------------------------
	// add mesh
	// ------------------------------------------------------
	void MeshBuffer::add(Mesh* mesh, const v3& position, const v3& scale, const v3& rotation, const Color& color) {
		mat4 w = matrix::mat4Transform(position);
		add(mesh, w, scale, rotation, color);
	}

	// ------------------------------------------------------
	// add mesh
	// ------------------------------------------------------
	void MeshBuffer::add(Mesh* mesh, const mat4& world, const v3& scale, const v3& rotation, const Color& color) {
		mat4 rotY = matrix::mat4RotationY(rotation.y);
		mat4 rotX = matrix::mat4RotationX(rotation.x);
		mat4 rotZ = matrix::mat4RotationZ(rotation.z);
		mat4 s = matrix::mat4Scale(scale);
		mat4 w = rotZ * rotY * rotX * s * world;
		for (int i = 0; i < mesh->vertices.size(); ++i) {
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
	void MeshBuffer::add(Mesh* mesh, const v3& position, const Color& color, const v3& scale, const v3& rotation) {
		mat4 rotY = matrix::mat4RotationY(rotation.y);
		mat4 rotX = matrix::mat4RotationX(rotation.x);
		mat4 rotZ = matrix::mat4RotationZ(rotation.z);
		mat4 t = matrix::mat4Transform(position);
		mat4 s = matrix::mat4Scale(scale);
		mat4 world = rotZ * rotY * rotX * s * t;
		for (int i = 0; i < mesh->vertices.size(); ++i) {
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
		_vertices.clear();
	}

	// ------------------------------------------------------
	// end
	// ------------------------------------------------------
	void MeshBuffer::end() {
		if (_vertices.size() > 0) {
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
		ZoneTracker("Mesh::drawImmediate");
		flush();

		mat4 w = matrix::m4identity();
		mat4 rotY = matrix::mat4RotationY(rotation.y);
		mat4 rotX = matrix::mat4RotationX(rotation.x);
		mat4 rotZ = matrix::mat4RotationZ(rotation.z);
		mat4 s = matrix::mat4Scale(scale);
		w = rotZ * rotY * rotX * s * world;
		unsigned int stride = sizeof(PNTCVertex);
		unsigned int offset = 0;

		graphics::setInputLayout(_descriptor.inputlayout);
		graphics::setVertexBuffer(_descriptor.vertexBuffer, &stride, &offset);
		graphics::setIndexBuffer(_descriptor.indexBuffer);
		graphics::setBlendState(_descriptor.blendstate);

		graphics::setShader(_descriptor.shader);
		graphics::setPixelShaderResourceView(_descriptor.colormap);

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
	}

	// ------------------------------------------------------
	// flush
	// ------------------------------------------------------
	void MeshBuffer::flush() {
		if (!_vertices.empty()) {
			ZoneTracker("Mesh::flush");

			mat4 world = matrix::m4identity();
			unsigned int stride = sizeof(PNTCVertex);
			unsigned int offset = 0;

			graphics::setInputLayout(_descriptor.inputlayout);
			graphics::setVertexBuffer(_descriptor.vertexBuffer, &stride, &offset);
			graphics::setIndexBuffer(_descriptor.indexBuffer);
			graphics::setBlendState(_descriptor.blendstate);

			graphics::setShader(_descriptor.shader);
			graphics::setPixelShaderResourceView(_descriptor.colormap);

			Camera* camera = graphics::getCamera();

			ds::mat4 mvp = world * camera->getViewProjectionMatrix();
			_buffer.viewProjectionMatrix = ds::matrix::mat4Transpose(mvp);
			_buffer.worldMatrix = ds::matrix::mat4Transpose(world);
			_buffer.cameraPos = camera->getPosition();
			_buffer.lightPos = _lightPos;
			_buffer.diffuseColor = _diffuseColor;
			graphics::mapData(_descriptor.vertexBuffer, _vertices.data(), _vertices.size() * sizeof(PNTCVertex));

			graphics::updateConstantBuffer(_descriptor.constantBuffer, &_buffer, sizeof(PNTCConstantBuffer));
			graphics::setVertexShaderConstantBuffer(_descriptor.constantBuffer);
			graphics::drawIndexed(_vertices.size() / 4 * 6);
			_vertices.clear();
		}
	}

}