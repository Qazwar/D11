#include "InputLayoutParser.h"

namespace ds {

	namespace res {

		struct InputElementDescriptor {

			const char* semantic;
			DXGI_FORMAT format;
			uint32_t size;

			InputElementDescriptor(const char* sem, DXGI_FORMAT f, uint32_t s) :
				semantic(sem), format(f), size(s) {
			}
		};

		static const InputElementDescriptor INPUT_ELEMENT_DESCRIPTIONS[] = {

			{ "POSITION", DXGI_FORMAT_R32G32B32_FLOAT, 12 },
			{ "COLOR", DXGI_FORMAT_R32G32B32A32_FLOAT, 16 },
			{ "TEXCOORD", DXGI_FORMAT_R32G32_FLOAT, 8 },
			{ "NORMAL", DXGI_FORMAT_R32G32B32_FLOAT, 12 }
		};



		// ------------------------------------------------------
		// find inputelement by name
		// ------------------------------------------------------
		static int findInputElement(const char* name) {
			for (int i = 0; i < 4; ++i) {
				if (strcmp(INPUT_ELEMENT_DESCRIPTIONS[i].semantic, name) == 0) {
					return i;
				}
			}
			return -1;
		}

		RID InputLayoutParser::createInputLayout(const char* name, const InputLayoutDescriptor& descriptor) {
			D3D11_INPUT_ELEMENT_DESC* descriptors = new D3D11_INPUT_ELEMENT_DESC[descriptor.num];
			uint32_t index = 0;
			uint32_t counter = 0;
			int si[8] = { 0 };
			for (int i = 0; i < descriptor.num; ++i) {
				const InputElementDescriptor& d = INPUT_ELEMENT_DESCRIPTIONS[descriptor.indices[i]];
				D3D11_INPUT_ELEMENT_DESC& desc = descriptors[i];
				desc.SemanticName = d.semantic;
				desc.SemanticIndex = si[descriptor.indices[i]];// d.semanticIndex;
				desc.Format = d.format;
				desc.InputSlot = 0;
				desc.AlignedByteOffset = index;
				desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				desc.InstanceDataStepRate = 0;
				index += d.size;
				si[descriptor.indices[i]] += 1;
			}
			ID3D11InputLayout* layout = 0;
			if (descriptor.shader == INVALID_RID) {
				_resCtx->device->CreateInputLayout(descriptors, descriptor.num, descriptor.byteCode, descriptor.byteCodeSize, &layout);
			}
			else {
				ShaderResource* sr = static_cast<ShaderResource*>(_resCtx->resources[descriptor.shader]);
				Shader* s = sr->get();
				assert(s != 0);
				HRESULT d3dResult = _resCtx->device->CreateInputLayout(descriptors, descriptor.num, s->vertexShaderBuffer->GetBufferPointer(), s->vertexShaderBuffer->GetBufferSize(), &layout);
				if (d3dResult < 0) {
					LOGE << "Cannot create input layout '" << name << "'";
					return INVALID_RID;
				}
			}
			delete[] descriptors;
			InputLayoutResource* ilr = new InputLayoutResource(layout, index);
			_resCtx->resources.push_back(ilr);
			return create(name, ResourceType::INPUTLAYOUT);
		}

		RID InputLayoutParser::parse(JSONReader& reader, int childIndex) {
			InputLayoutDescriptor descriptor;
			descriptor.num = 0;
			const char* attributes = reader.get_string(childIndex, "attributes");
			char buffer[256];
			sprintf_s(buffer, 256, "%s", attributes);
			char *token = token = strtok(buffer, ",");
			while (token) {
				int element = findInputElement(token);
				descriptor.indices[descriptor.num++] = element;
				token = strtok(NULL, ",");
			}
			const char* shaderName = reader.get_string(childIndex, "shader");
			descriptor.shader = find(shaderName, ResourceType::SHADER);
			const char* name = reader.get_string(childIndex, "name");
			return createInputLayout(name, descriptor);
		}
	}
}