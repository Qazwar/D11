#pragma once
#include "..\resources\ResourceDescriptors.h"
#include "VertexTypes.h"
#include "QuadBuffer.h"

namespace ds {

	class SkyBox {

	public:
		SkyBox(const SkyBoxDescriptor& descriptor);
		~SkyBox();
		void render();
	private:
		PNTCVertex _vertices[8];
		int _cubeIndices[36];
		PNTCConstantBuffer _buffer;
		Color _diffuseColor;
		SkyBoxDescriptor _descriptor;
	};

}

