#include "ConstantBufferParser.h"

namespace ds {

	namespace res {

		// ------------------------------------------------------
		// create quad buffer
		// ------------------------------------------------------
		RID ConstantBufferParser::createConsantBuffer(const char* name, const ConstantBufferDescriptor& descriptor) {
			D3D11_BUFFER_DESC constDesc;
			ZeroMemory(&constDesc, sizeof(constDesc));
			constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			constDesc.ByteWidth = descriptor.size;
			constDesc.Usage = D3D11_USAGE_DYNAMIC;
			constDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			ID3D11Buffer* buffer = 0;
			HRESULT d3dResult = _resCtx->device->CreateBuffer(&constDesc, 0, &buffer);
			if (FAILED(d3dResult))	{
				DXTRACE_MSG("Failed to create constant buffer!");
				return -1;
			}
			ConstantBufferResource* cbr = new ConstantBufferResource(buffer);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::CONSTANTBUFFER);
		}

		RID ConstantBufferParser::parse(JSONReader& reader, int childIndex) {
			ConstantBufferDescriptor descriptor;
			reader.get(childIndex, "size", &descriptor.size);
			const char* name = reader.get_string(childIndex, "name");
			return createConstantBuffer(name, descriptor);
		}
	}
}