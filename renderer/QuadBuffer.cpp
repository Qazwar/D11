#include "QuadBuffer.h"
#include "sprites.h"
#include "..\renderer\graphics.h"
#include <assert.h>
#include "core\math\math_types.h"
#include "core\math\matrix.h"
#include "..\resources\ResourceDescriptors.h"
#include "..\resources\ResourceContainer.h"
#include "core\log\Log.h"
#include "core\profiler\Profiler.h"

namespace ds {
	
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
		if (_index > 0) {
			ZoneTracker("QuadBuffer::flush");
			unsigned int stride = sizeof(PNTCVertex);
			unsigned int offset = 0;

			//graphics::setInputLayout(_descriptor.inputlayout);
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

			graphics::updateConstantBuffer(_descriptor.constantBuffer, &mvp, sizeof(mat4));

			graphics::setVertexShaderConstantBuffer(_descriptor.constantBuffer);
			graphics::drawIndexed(_index / 4 * 6);

			_index = 0;
		}
	}

}