#include "GrayFadePostProcess.h"
#include "..\renderer\graphics.h"
#include "..\utils\Profiler.h"
#include "..\resources\ResourceContainer.h"
#include "..\shaders\postprocess\GrayFade_PS_Main.inc"
#include "..\shaders\postprocess\GrayFade_VS_Main.inc"

namespace ds {

	GrayFadePostProcess::GrayFadePostProcess(const GrayFadePostProcessDescriptor& descriptor) : PostProcess(descriptor), _descriptor(descriptor), _timer(0.0f) {
		assert(_descriptor.ttl > 0.0f);

		ds::ConstantBufferDescriptor cbDesc;
		cbDesc.size = 16;
		_cbID = ds::res::createConstantBuffer("GrayFadeCB", cbDesc);

		RID ss_id = ds::res::find("SpriteSampler", ResourceType::SAMPLERSTATE);
		RID shader_id = ds::res::createEmptyShader("GrayFade");
		ds::Shader* s = ds::res::getShader(shader_id);
		graphics::getDevice()->CreateVertexShader(GrayFade_VS_Main, sizeof(GrayFade_VS_Main), 0, &s->vertexShader);
		graphics::getDevice()->CreatePixelShader(GrayFade_PS_Main, sizeof(GrayFade_PS_Main), 0, &s->pixelShader);
		s->samplerState = ds::res::getSamplerState(ss_id);

		ds::MaterialDescriptor mtrlDesc;
		mtrlDesc.shader = shader_id;
		mtrlDesc.blendstate = ds::res::find("DefaultBlendState",ResourceType::BLENDSTATE);
		mtrlDesc.texture = INVALID_RID;
		mtrlDesc.renderTarget = _descriptor.source;
		_material = ds::res::createMaterial("GrayFadeMaterial", mtrlDesc);
	}


	GrayFadePostProcess::~GrayFadePostProcess() {
	}

	void GrayFadePostProcess::updateConstantBuffer() {
		_cbData.x = math::clamp(_timer / _descriptor.ttl,0.0f,1.0f);
		graphics::updateConstantBuffer(_cbID, &_cbData, sizeof(v4));
		graphics::setPixelShaderConstantBuffer(_cbID);
	}

	void GrayFadePostProcess::tick(float dt) {
		if (_active) {
			_timer += dt;

		}
	}

}