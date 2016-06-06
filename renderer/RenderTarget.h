#pragma once
#include "..\resources\ResourceDescriptors.h"

namespace ds {

	class RenderTarget {

	public:
		RenderTarget(const RenderTargetDescriptor& descriptor);
		~RenderTarget();
	};

}