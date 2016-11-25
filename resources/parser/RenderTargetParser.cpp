#include "RenderTargetParser.h"
#include <core\base\Assert.h>

namespace ds {

	namespace res {

		RID RenderTargetParser::createRenderTarget(const char* name, const RenderTargetDescriptor& descriptor) {
			RenderTarget* rt = new RenderTarget(descriptor);
			bool ret = rt->init(graphics::getDevice(), descriptor.width, descriptor.height);
			XASSERT(ret, "Could not create rendertarget: %s", name);
			RenderTargetResource* cbr = new RenderTargetResource(rt);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::RENDERTARGET);
		}

		RID RenderTargetParser::parse(JSONReader& reader, int childIndex) {
			RenderTargetDescriptor descriptor;
			reader.get(childIndex, "width", &descriptor.width);
			reader.get(childIndex, "height", &descriptor.height);
			reader.get(childIndex, "clear_color", &descriptor.clearColor);
			const char* name = reader.get_string(childIndex, "name");
			return createRenderTarget(name, descriptor);
		}
	}
}