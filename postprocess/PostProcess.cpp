#include "PostProcess.h"
#include "..\renderer\graphics.h"
#include "core\profiler\Profiler.h"
#include "..\renderer\VertexTypes.h"
#include "..\resources\ResourceContainer.h"

namespace ds {

	PostProcess::PostProcess(const PostProcessDescriptor& descriptor) : _active(false), _target(descriptor.target), _source(descriptor.source) {
		_vertexBuffer = ds::res::find("PostProcessVertexBuffer", ds::ResourceType::VERTEXBUFFER);
	}

	void PostProcess::render() {
		ZoneTracker("PostProcess::render");
		if (_target != INVALID_RID) {
			// set render target
			graphics::setRenderTarget(_target);
		}
		unsigned int stride = sizeof(PTCVertex);
		unsigned int offset = 0;
		graphics::turnOffZBuffer();
		graphics::setVertexBuffer(_vertexBuffer, &stride, &offset, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		graphics::setMaterial(_material);
		updateConstantBuffer();
		graphics::draw(6);
		graphics::turnOnZBuffer();
		if (_target != INVALID_RID) {
			// restore back buffer
			graphics::restoreBackbuffer();
		}
	}
}