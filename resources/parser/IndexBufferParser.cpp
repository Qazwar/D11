#include "IndexBufferParser.h"

namespace ds {

	namespace res {

		RID IndexBufferParser::createIndexBuffer(const char* name, const IndexBufferDescriptor& descriptor) {
			D3D11_BUFFER_DESC bufferDesc;
			if (descriptor.dynamic) {
				bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
				bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			}
			else {
				bufferDesc.Usage = D3D11_USAGE_DEFAULT;
				bufferDesc.CPUAccessFlags = 0;
			}
			bufferDesc.ByteWidth = sizeof(uint32_t) * descriptor.size;
			bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bufferDesc.MiscFlags = 0;
			ID3D11Buffer* buffer;
			HRESULT hr = _resCtx->device->CreateBuffer(&bufferDesc, 0, &buffer);
			if (FAILED(hr)) {
				DXTRACE_MSG("Failed to create index buffer!");
				return -1;
			}
			IndexBufferResource* cbr = new IndexBufferResource(buffer);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::INDEXBUFFER);
		}

		RID IndexBufferParser::parse(JSONReader& reader, int childIndex) {
			IndexBufferDescriptor descriptor;
			reader.get(childIndex, "size", &descriptor.size);
			const char* name = reader.get_string(childIndex, "name");
			return createIndexBuffer(name, descriptor);
		}
	}
}