#include "SamplerStateParser.h"
#include <core\base\Assert.h>

namespace ds {

	namespace res {

		struct TextureAddressModeMapping {
			const char* name;
			D3D11_TEXTURE_ADDRESS_MODE mode;
		};

		static const TextureAddressModeMapping TEXTURE_ADDRESS_MODES[] = {
			{ "WRAP", D3D11_TEXTURE_ADDRESS_WRAP },
			{ "MIRROR", D3D11_TEXTURE_ADDRESS_MIRROR },
			{ "CLAMP", D3D11_TEXTURE_ADDRESS_CLAMP },
			{ "BORDER", D3D11_TEXTURE_ADDRESS_BORDER },
			{ "MIRROR_ONCE", D3D11_TEXTURE_ADDRESS_MIRROR_ONCE },
		};

		// ------------------------------------------------------
		// find sampler state by name
		// ------------------------------------------------------
		static int findTextureAddressMode(const char* name) {
			for (int i = 0; i < 5; ++i) {
				if (strcmp(TEXTURE_ADDRESS_MODES[i].name, name) == 0) {
					return i;
				}
			}
			return -1;
		}

		RID SamplerStateParser::createSamplerState(const char* name, const SamplerStateDescriptor& descriptor) {
			D3D11_SAMPLER_DESC colorMapDesc;
			ZeroMemory(&colorMapDesc, sizeof(colorMapDesc));
			colorMapDesc.AddressU = TEXTURE_ADDRESS_MODES[descriptor.addressU].mode;
			colorMapDesc.AddressV = TEXTURE_ADDRESS_MODES[descriptor.addressV].mode;
			colorMapDesc.AddressW = TEXTURE_ADDRESS_MODES[descriptor.addressW].mode;
			colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			//colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			//colorMapDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;
			ID3D11SamplerState* sampler;
			HRESULT d3dResult = _resCtx->device->CreateSamplerState(&colorMapDesc, &sampler);
			if (FAILED(d3dResult)) {
				DXTRACE_MSG("Failed to create SamplerState!");
				return INVALID_RID;
			}
			SamplerStateResource* cbr = new SamplerStateResource(sampler);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::SAMPLERSTATE);
		}

		RID SamplerStateParser::parse(JSONReader& reader, int childIndex) {
			SamplerStateDescriptor descriptor;
			const char* mode = reader.get_string(childIndex, "addressU");
			int idx = findTextureAddressMode(mode);
			XASSERT(idx >= 0, "No matching address mode for addressU mode: '%s'", mode);
			descriptor.addressU = idx;
			mode = reader.get_string(childIndex, "addressV");
			idx = findTextureAddressMode(mode);
			XASSERT(idx >= 0, "No matching address mode for addressV mode: '%s'", mode);
			descriptor.addressV = idx;
			mode = reader.get_string(childIndex, "addressW");
			idx = findTextureAddressMode(mode);
			XASSERT(idx >= 0, "No matching address mode for addressW mode: '%s'", mode);
			descriptor.addressW = idx;
			const char* name = reader.get_string(childIndex, "name");
			return createSamplerState(name, descriptor);
		}
	}
}