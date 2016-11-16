#include "SquareBuffer.h"
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
	
	SquareBuffer::SquareBuffer(const SquareBufferDescriptor& descriptor) : _descriptor(descriptor), _index(0), _started(false) {
		// create data
		_maxSprites = descriptor.size;
		_vertices = new QuadVertex[4 * descriptor.size];
		_constantBuffer.setScreenSize(v2(graphics::getScreenWidth(), graphics::getScreenHeight()));
		_constantBuffer.setTextureSize(1024.0f, 1024.0f);
	}

	SquareBuffer::~SquareBuffer() {
		delete[] _vertices;
	}

	void SquareBuffer::draw(v3* positions, const Texture& t, const Color& color) {
		if (_started) {
			if ((_index + 4) >= _maxSprites) {
				flush();
			}
			for (int i = 0; i < 4; ++i) {
				_vertices[_index++] = QuadVertex(positions[i], t.uv[i], color);
			}
		}
	}

	void SquareBuffer::draw(const v3& position, const v2& uv, const Color& color) {
		if (_started) {
			if (_index >= _maxSprites) {
				flush();
			}
			_vertices[_index++] = QuadVertex(position, uv, color);
		}
	}
	
	void SquareBuffer::begin() {
		_index = 0;
		_started = true;
		_currentMtrl = _descriptor.material;
	}

	void SquareBuffer::end() {
		flush();
		_started = false;
	}

	void SquareBuffer::flush() {
		if (_index > 0) {
			ZoneTracker("SquareBuffer::flush");
			unsigned int stride = sizeof(QuadVertex);
			unsigned int offset = 0;
			unsigned int idx = _index / 4 * 6;
			graphics::turnOffZBuffer();
			graphics::setIndexBuffer(_descriptor.indexBuffer);
			graphics::setVertexBuffer(_descriptor.vertexBuffer, &stride, &offset, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			graphics::setMaterial(_currentMtrl);
			graphics::mapData(_descriptor.vertexBuffer, _vertices, _index * sizeof(QuadVertex));
			mat4 w = matrix::m4identity();
			_constantBuffer.wvp = ds::matrix::mat4Transpose(w * graphics::getOrthoCamera()->getViewProjectionMatrix());
			graphics::updateConstantBuffer(_descriptor.constantBuffer,&_constantBuffer,sizeof(SpriteBufferCB));
			graphics::drawIndexed(idx);
			graphics::turnOnZBuffer();
			//gDrawCounter->sprites += _index;
			_index = 0;
		}
	}

}