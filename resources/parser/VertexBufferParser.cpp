#include "VertexBufferParser.h"

namespace ds {

	namespace res {

		RID VertexBufferParser::createVertexBuffer(const char* name, const VertexBufferDescriptor& descriptor) {
			InputLayoutResource* res = static_cast<InputLayoutResource*>(_resCtx->resources[descriptor.layout]);
			UINT size = descriptor.size * res->size();

			D3D11_BUFFER_DESC bufferDesciption;
			ZeroMemory(&bufferDesciption, sizeof(bufferDesciption));
			if (descriptor.dynamic) {
				bufferDesciption.Usage = D3D11_USAGE_DYNAMIC;
				bufferDesciption.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			}
			else {
				bufferDesciption.Usage = D3D11_USAGE_DEFAULT;
				bufferDesciption.CPUAccessFlags = 0;
			}
			bufferDesciption.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDesciption.ByteWidth = size;

			ID3D11Buffer* buffer = 0;
			if (descriptor.dataSize == 0) {
				HRESULT d3dResult = _resCtx->device->CreateBuffer(&bufferDesciption, 0, &buffer);
				if (FAILED(d3dResult))	{
					DXTRACE_MSG("Failed to create buffer!");
					return -1;
				}
			}
			else {
				D3D11_SUBRESOURCE_DATA resource;
				resource.pSysMem = descriptor.data;
				resource.SysMemPitch = 0;
				resource.SysMemSlicePitch = 0;
				HRESULT d3dResult = _resCtx->device->CreateBuffer(&bufferDesciption, &resource, &buffer);
				if (FAILED(d3dResult))	{
					DXTRACE_MSG("Failed to create buffer!");
					return -1;
				}
			}
			int idx = _resCtx->resources.size();
			VertexBufferResource* cbr = new VertexBufferResource(buffer, size, descriptor.layout);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::VERTEXBUFFER);
		}

		RID VertexBufferParser::parse(JSONReader& reader, int childIndex) {
			VertexBufferDescriptor descriptor;
			reader.get(childIndex, "size", &descriptor.size);
			reader.get(childIndex, "dynamic", &descriptor.dynamic);
			const char* layoutName = reader.get_string(childIndex, "layout");
			descriptor.layout = find(layoutName, ResourceType::INPUTLAYOUT);
			const char* name = reader.get_string(childIndex, "name");
			return createVertexBuffer(name, descriptor);
		}
	}
}