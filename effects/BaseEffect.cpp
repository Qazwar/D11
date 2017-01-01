#include "BaseEffect.h"

void ds::FullScreenEffect::end() {
	ZoneTracker("FullScreenEffect::render");
	if (_target != INVALID_RID) {
		// set render target
		graphics::setRenderTarget(_target);
	}
	else {
		graphics::restoreBackbuffer();
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
