#include "ParticleManagerParser.h"

namespace ds {

	namespace res {

		RID ParticleManagerParser::createParticleManager(const ParticleSystemsDescriptor& descriptor) {
			_resCtx->particles = new ParticleManager(descriptor);
			_resCtx->particles->load();
			int idx = _resCtx->resources.size();
			ParticleManagerResource* cbr = new ParticleManagerResource(_resCtx->particles);
			_resCtx->resources.push_back(cbr);
			return create("ParticleManager", ResourceType::PARTICLEMANAGER);
		}

		RID ParticleManagerParser::parse(JSONReader& reader, int childIndex) {
			ParticleSystemsDescriptor descriptor;
			reader.get(childIndex, "sprite_buffer", &descriptor.spriteBuffer);
			const char* name = reader.get_string(childIndex, "name");
			return createParticleManager(descriptor);
		}
	}
}