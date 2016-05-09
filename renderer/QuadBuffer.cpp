#include "QuadBuffer.h"
#include "sprites.h"
#include "..\renderer\graphics.h"
#include <assert.h>
#include "..\math\math_types.h"
#include "..\math\matrix.h"
#include "..\resources\ResourceDescriptors.h"
#include "..\resources\ResourceContainer.h"
#include "..\utils\Log.h"
#include "..\utils\Profiler.h"

namespace ds {

	Mesh::Mesh(const MeshDescriptor& descriptor) : _descriptor(descriptor) {
		_position = v3(0, 0, 0);
		_scale = v3(1, 1, 1);
		_rotation = v3(0, 0, 0);
		_lightPos = v3(0.0f, 300.0f, -50.0f);
	}

	Mesh::~Mesh() {

	}

	void Mesh::add(const v3& position, const v3& normal, const v2& uv, const Color& color) {
		_vertices.push_back(PNTCVertex(position, normal, uv, color));
	}

	void Mesh::add(const PNTCVertex& v) {
		_vertices.push_back(v);
	}

	void Mesh::rotateY(float angle) {
		_rotation.y = angle;
	}

	void Mesh::rotateX(float angle) {
		_rotation.x = angle;
	}

	void Mesh::rotateZ(float angle) {
		_rotation.z = angle;
	}

	void Mesh::translate(const v3& position) {
		_position = position;
	}

	void Mesh::scale(const v3& scale) {
		_scale = scale;
	}

	void Mesh::draw() {
		ZoneTracker("Mesh::draw");

		mat4 world = matrix::m4identity();
		mat4 rotY = matrix::mat4RotationY(_rotation.y);
		mat4 rotX = matrix::mat4RotationX(_rotation.x);
		mat4 rotZ = matrix::mat4RotationZ(_rotation.z);
		mat4 t = matrix::mat4Transform(_position);
		mat4 s = matrix::mat4Scale(_scale);
		world = s * rotZ * rotY * rotX * t;
		unsigned int stride = sizeof(PNTCVertex);
		unsigned int offset = 0;

		graphics::setInputLayout(_descriptor.inputlayout);
		graphics::setVertexBuffer(_descriptor.vertexBuffer, &stride, &offset);
		graphics::setIndexBuffer(_descriptor.indexBuffer);
		graphics::setBlendState(_descriptor.blendstate);

		graphics::setShader(_descriptor.shader);
		graphics::setPixelShaderResourceView(_descriptor.colormap);

		Camera* camera = graphics::getCamera();

		//ds::mat4 mvp = graphics::getViewProjectionMaxtrix();
		ds::mat4 mvp = world * camera->getViewProjectionMatrix();
		_buffer.viewProjectionMatrix = ds::matrix::mat4Transpose(mvp);
		_buffer.worldMatrix = ds::matrix::mat4Transpose(world);
		_buffer.cameraPos = camera->getTarget() - camera->getPosition();
		_buffer.lightPos = _lightPos;

		graphics::mapData(_descriptor.vertexBuffer, _vertices.data(), _vertices.size() * sizeof(PNTCVertex));

		graphics::updateConstantBuffer(_descriptor.constantBuffer, &_buffer);
		graphics::setVertexShaderConstantBuffer(_descriptor.constantBuffer);
		graphics::drawIndexed(_vertices.size() / 4 * 6);
	}




	QuadBuffer::QuadBuffer(const QuadBufferDescriptor& descriptor) : _descriptor(descriptor), _index(0), _started(false) {
		// create data
		_maxSprites = descriptor.size;
		_vertices = new QuadVertex[4 * descriptor.size];
	}

	QuadBuffer::~QuadBuffer() {
		delete[] _vertices;
	}

	void QuadBuffer::draw(const v3& position, const v2& uv, const Color& color) {
		if (_started) {
			if (_index >= _maxSprites) {
				flush();
			}
			_vertices[_index++] = QuadVertex(position, uv, color);
		}
	}
	
	void QuadBuffer::begin() {
		_index = 0;
		_started = true;
	}

	void QuadBuffer::end() {
		flush();
		_started = false;
	}

	void QuadBuffer::flush() {
		ZoneTracker("QuadBuffer::flush");
		unsigned int stride = sizeof(PNTCVertex);
		unsigned int offset = 0;

		graphics::setInputLayout(_descriptor.inputlayout);
		graphics::setVertexBuffer(_descriptor.vertexBuffer, &stride, &offset);
		graphics::setIndexBuffer(_descriptor.indexBuffer);
		graphics::setBlendState(_descriptor.blendstate);

		graphics::setShader(_descriptor.shader);
		graphics::setPixelShaderResourceView(_descriptor.colormap);

		Camera* camera = graphics::getCamera();

		//ds::mat4 mvp = graphics::getViewProjectionMaxtrix();
		ds::mat4 mvp = camera->getViewProjectionMatrix();
		mvp = ds::matrix::mat4Transpose(mvp);

		graphics::mapData(_descriptor.vertexBuffer, _vertices, _index * sizeof(PNTCVertex));

		graphics::updateConstantBuffer(_descriptor.constantBuffer, &mvp);
		
		graphics::setVertexShaderConstantBuffer(_descriptor.constantBuffer);
		graphics::drawIndexed(_index / 4 * 6);

		_index = 0;
	}

}