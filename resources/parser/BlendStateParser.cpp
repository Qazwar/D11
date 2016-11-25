#include "BlendStateParser.h"

namespace ds {

	namespace res {

		struct BlendStateMapping {
			const char* name;
			D3D11_BLEND blend;
			BlendStateMapping(const char* n, D3D11_BLEND b) : name(n), blend(b) {}
		};

		static const BlendStateMapping BLEND_STATE_MAPPINGS[] = {
			{ "ZERO", D3D11_BLEND_ZERO },
			{ "ONE", D3D11_BLEND_ONE },
			{ "SRC_COLOR", D3D11_BLEND_SRC_COLOR },
			{ "INV_SRC_COLOR", D3D11_BLEND_INV_SRC_COLOR },
			{ "SRC_ALPHA", D3D11_BLEND_SRC_ALPHA },
			{ "INV_SRC_ALPHA", D3D11_BLEND_INV_SRC_ALPHA },
			{ "DEST_ALPHA", D3D11_BLEND_DEST_ALPHA },
			{ "INV_DEST_ALPHA", D3D11_BLEND_INV_DEST_ALPHA },
			{ "DEST_COLOR", D3D11_BLEND_DEST_COLOR },
			{ "INV_DEST_COLOR", D3D11_BLEND_INV_DEST_COLOR },
			{ "SRC_ALPHA_SAT", D3D11_BLEND_SRC_ALPHA_SAT },
			{ "BLEND_FACTOR", D3D11_BLEND_BLEND_FACTOR },
			{ "INV_BLEND_FACTOR", D3D11_BLEND_INV_BLEND_FACTOR },
			{ "SRC1_COLOR", D3D11_BLEND_SRC1_COLOR },
			{ "INV_SRC1_COLOR", D3D11_BLEND_INV_SRC1_COLOR },
			{ "SRC1_ALPHA", D3D11_BLEND_SRC1_ALPHA },
			{ "INV_SRC1_ALPHA", D3D11_BLEND_INV_SRC1_ALPHA },
		};

		// ------------------------------------------------------
		// find blendstate by name
		// ------------------------------------------------------
		int findBlendStateMapping(const char* text) {
			for (int i = 0; i < 17; ++i) {
				if (strcmp(BLEND_STATE_MAPPINGS[i].name, text) == 0) {
					return i;
				}
			}
			return -1;
		}

		// ------------------------------------------------------
		// create blend state
		// ------------------------------------------------------
		RID BlendStateParser::createBlendState(const char* name, const BlendStateDescriptor& descriptor) {
			D3D11_BLEND_DESC blendDesc;
			ZeroMemory(&blendDesc, sizeof(blendDesc));
			if (descriptor.alphaEnabled) {
				blendDesc.RenderTarget[0].BlendEnable = TRUE;
			}
			else {
				blendDesc.RenderTarget[0].BlendEnable = FALSE;
				//blendDesc.RenderTarget[0].BlendEnable = (srcBlend != D3D11_BLEND_ONE) || (destBlend != D3D11_BLEND_ZERO);
			}
			blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlend = BLEND_STATE_MAPPINGS[descriptor.srcBlend].blend;
			blendDesc.RenderTarget[0].DestBlend = BLEND_STATE_MAPPINGS[descriptor.destBlend].blend;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlendAlpha = BLEND_STATE_MAPPINGS[descriptor.srcAlphaBlend].blend;
			blendDesc.RenderTarget[0].DestBlendAlpha = BLEND_STATE_MAPPINGS[descriptor.destAlphaBlend].blend;
			blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;

			ID3D11BlendState* state;
			HRESULT d3dResult = _resCtx->device->CreateBlendState(&blendDesc, &state);
			if (FAILED(d3dResult)) {
				DXTRACE_MSG("Failed to create blendstate!");
				return INVALID_RID;
			}
			BlendStateResource* cbr = new BlendStateResource(state);
			_resCtx->resources.push_back(cbr);
			return create(name, ResourceType::BLENDSTATE);
		}

		RID BlendStateParser::parse(JSONReader& reader, int childIndex) {
			BlendStateDescriptor descriptor;
			const char* entry = reader.get_string(childIndex, "src_blend");
			descriptor.srcBlend = findBlendStateMapping(entry);
			entry = reader.get_string(childIndex, "dest_blend");
			descriptor.destBlend = findBlendStateMapping(entry);
			entry = reader.get_string(childIndex, "src_blend_alpha");
			descriptor.srcAlphaBlend = findBlendStateMapping(entry);
			entry = reader.get_string(childIndex, "dest_blend_alpha");
			descriptor.destAlphaBlend = findBlendStateMapping(entry);
			reader.get(childIndex, "alpha_enabled", &descriptor.alphaEnabled);
			const char* name = reader.get_string(childIndex, "name");
			return createBlendState(name, descriptor);
		}
	}
}