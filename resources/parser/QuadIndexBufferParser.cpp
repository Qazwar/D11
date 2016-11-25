#include "QuadIndexBufferParser.h"

namespace ds {

	namespace res {

		// ------------------------------------------------------
		// cretate quad index buffer
		// ------------------------------------------------------
		RID QuadIndexBufferParser::createQuadIndexBuffer(const char* name, const QuadIndexBufferDescriptor& descriptor) {

			// FIXME: check that size % 6 == 0 !!!
			D3D11_BUFFER_DESC bufferDesc;
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.CPUAccessFlags = 0;
			bufferDesc.ByteWidth = sizeof(uint32_t) * descriptor.size;
			bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bufferDesc.MiscFlags = 0;

			uint32_t* data = new uint32_t[descriptor.size];
			int base = 0;
			int cnt = 0;
			int num = descriptor.size / 6;
			for (int i = 0; i < num; ++i) {
				data[base] = cnt;
				data[base + 1] = cnt + 1;
				data[base + 2] = cnt + 3;
				data[base + 3] = cnt + 1;
				data[base + 4] = cnt + 2;
				data[base + 5] = cnt + 3;
				base += 6;
				cnt += 4;
			}
			// Define the resource data.
			D3D11_SUBRESOURCE_DATA InitData;
			InitData.pSysMem = data;
			InitData.SysMemPitch = 0;
			InitData.SysMemSlicePitch = 0;
			// Create the buffer with the device.
			ID3D11Buffer* buffer;
			HRESULT hr = _resCtx->device->CreateBuffer(&bufferDesc, &InitData, &buffer);
			if (FAILED(hr)) {
				delete[] data;
				DXTRACE_MSG("Failed to create quad index buffer!");
				return -1;
			}
			delete[] data;
			IndexBufferResource* cbr = new IndexBufferResource(buffer);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::INDEXBUFFER);
		}

		RID QuadIndexBufferParser::parse(JSONReader& reader, int childIndex) {
			QuadIndexBufferDescriptor descriptor;
			reader.get(childIndex, "size", &descriptor.size);
			const char* name = reader.get_string(childIndex, "name");
			return createQuadIndexBuffer(name, descriptor);
		}
	}
}