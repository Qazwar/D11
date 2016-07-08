#include "ParticleSystemRenderer.h"
#include "..\resources\ResourceContainer.h"

namespace ds {

	ParticleSystemRenderer2D::ParticleSystemRenderer2D(RID bufferID) : ParticleSystemRenderer() {
		_particles = res::getSpriteBuffer(bufferID);
	}
	

	void ParticleSystemRenderer2D::render(const ParticleArray& array, const Texture& t) {
		_particles->begin();
		int batchSize = 0;
		_particles->begin();
		if (array.countAlive > 0) {
			for (int j = 0; j < array.countAlive; ++j) {
				_particles->draw(array.position[j].xy(), t, array.rotation[j], array.scale[j], array.color[j]);
			}
		}
		_particles->end();
	}

}