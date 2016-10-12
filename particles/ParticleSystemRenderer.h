#pragma once
#include "Particle.h"
#include "..\renderer\sprites.h"

namespace ds {

	class ParticleSystemRenderer {

	public:
		ParticleSystemRenderer() {}
		virtual ~ParticleSystemRenderer() {}

		virtual void render(const ParticleArray& array, const Texture& t) = 0;
		virtual void end() = 0;
	};

	class ParticleSystemRenderer2D : public ParticleSystemRenderer {

	public:
		ParticleSystemRenderer2D(RID bufferID);
		virtual ~ParticleSystemRenderer2D() {}

		void render(const ParticleArray& array, const Texture& t);
		void end() {}

	private:
		SpriteBuffer* _particles;
	};

}