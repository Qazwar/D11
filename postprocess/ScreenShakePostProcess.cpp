#include "ScreenShakePostProcess.h"
#include "..\renderer\graphics.h"
#include "core\profiler\Profiler.h"
#include "..\resources\ResourceContainer.h"
#include "..\shaders\postprocess\ScreenShake_PS_Main.inc"
#include "..\shaders\postprocess\ScreenShake_VS_Main.inc"

namespace ds {

	ScreenShakePostProcess::ScreenShakePostProcess(const ScreenShakePostProcessDescriptor& descriptor) : PostProcess(descriptor), _descriptor(descriptor) {
		assert(_descriptor.ttl > 0.0f);

		ds::ConstantBufferDescriptor cbDesc;
		cbDesc.size = 16;
		_cbID = ds::res::createConstantBuffer("ScreenShakeCB", cbDesc);

		RID ss_id = ds::res::find("SpriteSampler", ResourceType::SAMPLERSTATE);
		RID shader_id = ds::res::createEmptyShader("ScreenShake");
		ds::Shader* s = ds::res::getShader(shader_id);
		graphics::getDevice()->CreateVertexShader(ScreenShake_VS_Main, sizeof(ScreenShake_VS_Main), 0, &s->vertexShader);
		graphics::getDevice()->CreatePixelShader(ScreenShake_PS_Main, sizeof(ScreenShake_PS_Main), 0, &s->pixelShader);
		s->samplerState = ds::res::getSamplerState(ss_id);

		ds::MaterialDescriptor mtrlDesc;
		mtrlDesc.shader = shader_id;
		mtrlDesc.blendstate = ds::res::find("DefaultBlendState",ResourceType::BLENDSTATE);
		mtrlDesc.texture = INVALID_RID;
		mtrlDesc.renderTarget = _source;
		_material = ds::res::createMaterial("ScreenShakeMaterial", mtrlDesc);

	}

	ScreenShakePostProcess::~ScreenShakePostProcess() {
	}

	void ScreenShakePostProcess::updateConstantBuffer() {
		_constantBuffer.period = 1.0f / _descriptor.ttl;
		float n = _constantBuffer.timer / _descriptor.ttl;
		_constantBuffer.shakeAmount = _descriptor.shakeAmount / 1024.0f * cos(n * HALF_PI);
		_constantBuffer.frequency = _descriptor.frequency;
		graphics::updateConstantBuffer(_cbID, &_constantBuffer, sizeof(ScreenShakeConstantBuffer));
		graphics::setPixelShaderConstantBuffer(_cbID);
	}

	void ScreenShakePostProcess::tick(float dt) {
		if (_active) {
			_constantBuffer.timer += dt;
			float n = _constantBuffer.timer / _descriptor.ttl;
			if (n > 1.0f) {
				_active = false;
			}

		}
	}

}