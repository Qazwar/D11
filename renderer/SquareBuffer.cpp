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
#include "..\stats\DrawCounter.h"

namespace ds {
	
	SquareBuffer::SquareBuffer(const SquareBufferDescriptor& descriptor) : _descriptor(descriptor), _index(0), _started(false) {
		// create data
		_maxSprites = descriptor.size;
		_vertices = new QuadVertex[descriptor.size];
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

	void SquareBuffer::drawLine(const v3& start, const v3& end, float thickness, const Texture& t, const Color& color) {
		if (_started) {
			if ((_index + 4) >= _maxSprites) {
				flush();
			}
			v3 p[4];
			v3 d = end - start;
			v3 dn = normalize(d);
			v3 rn = v3(-dn.y, dn.x, 0.0f);
			p[0] = end + rn * thickness * 0.5f;
			p[1] = end - rn * thickness * 0.5f;
			p[2] = start - rn * thickness * 0.5f;
			p[3] = start + rn * thickness * 0.5f;
			draw(p, t, color);
		}
	}

	void SquareBuffer::drawLine(const v3& start, const v3& end, const v3& offset, const Texture& t, const Color& color) {
		if (_started) {
			if ((_index + 4) >= _maxSprites) {
				flush();
			}
			v3 p[4];
			p[0] = end - offset;
			p[1] = end + offset;
			p[2] = start + offset;
			p[3] = start - offset;
			draw(p, t, color);
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
			gDrawCounter->squares += _index;
			gDrawCounter->flushes += 1;
			_index = 0;
		}
	}

}