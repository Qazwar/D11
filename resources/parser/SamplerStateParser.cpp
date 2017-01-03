#include "SamplerStateParser.h"
#include <core\base\Assert.h>

namespace ds {

	namespace res {

		struct TextureAddressModeMapping {
			const char* name;
			D3D11_TEXTURE_ADDRESS_MODE mode;
		};

		struct TextureFilterMapping {
			const char* name;
			D3D11_FILTER filter;
		};

		static const TextureAddressModeMapping TEXTURE_ADDRESS_MODES[] = {
			{ "WRAP", D3D11_TEXTURE_ADDRESS_WRAP },
			{ "MIRROR", D3D11_TEXTURE_ADDRESS_MIRROR },
			{ "CLAMP", D3D11_TEXTURE_ADDRESS_CLAMP },
			{ "BORDER", D3D11_TEXTURE_ADDRESS_BORDER },
			{ "MIRROR_ONCE", D3D11_TEXTURE_ADDRESS_MIRROR_ONCE },
		};

		static const TextureFilterMapping TEXTURE_FILTER_MODES[] = {
			{ "POINT", D3D11_FILTER_MIN_MAG_MIP_POINT },
			{ "LINEAR", D3D11_FILTER_MIN_MAG_MIP_LINEAR },
			{ "ANISOTROPIC", D3D11_FILTER_ANISOTROPIC },
		};

		// ------------------------------------------------------
		// find sampler state by name
		// ------------------------------------------------------
		static D3D11_TEXTURE_ADDRESS_MODE findTextureAddressMode(const char* name) {
			for (int i = 0; i < 5; ++i) {
				if (strcmp(TEXTURE_ADDRESS_MODES[i].name, name) == 0) {
					return TEXTURE_ADDRESS_MODES[i].mode;
				}
			}
			return D3D11_TEXTURE_ADDRESS_CLAMP;
		}

		// ------------------------------------------------------
		// find filter state by name
		// ------------------------------------------------------
		static D3D11_FILTER findTextureFilterMode(const char* name) {
			for (int i = 0; i < 3; ++i) {
				if (strcmp(TEXTURE_FILTER_MODES[i].name, name) == 0) {
					return TEXTURE_FILTER_MODES[i].filter;
				}
			}
			return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		}

		RID SamplerStateParser::createSamplerState(const char* name, const SamplerStateDescriptor& descriptor) {
			D3D11_SAMPLER_DESC colorMapDesc;
			ZeroMemory(&colorMapDesc, sizeof(colorMapDesc));
			
			colorMapDesc.AddressU = findTextureAddressMode(descriptor.addressU);
			colorMapDesc.AddressV = findTextureAddressMode(descriptor.addressV);
			colorMapDesc.AddressW = findTextureAddressMode(descriptor.addressW);

			colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			colorMapDesc.Filter = findTextureFilterMode(descriptor.filter);
			//colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
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
			descriptor.addressU = reader.get_string(childIndex, "addressU");
			descriptor.addressV = reader.get_string(childIndex, "addressV");
			descriptor.addressW = reader.get_string(childIndex, "addressW");
			descriptor.filter = reader.get_string(childIndex, "filter");
			const char* name = reader.get_string(childIndex, "name");
			return createSamplerState(name, descriptor);
		}
	}
}