#include "ScreenShakePostProcess.h"
#include "..\renderer\graphics.h"
#include "..\utils\Profiler.h"
#include "..\resources\ResourceContainer.h"
#include "..\shaders\postprocess\ScreenShake_PS_Main.inc"
#include "..\shaders\postprocess\ScreenShake_VS_Main.inc"

namespace ds {

	ScreenShakePostProcess::ScreenShakePostProcess(const ScreenShakePostProcessDescriptor& descriptor) : PostProcess() , _descriptor(descriptor) {
		assert(_descriptor.ttl > 0.0f);

		_vertexBuffer = ds::res::find("PostProcessVertexBuffer",ResourceType::VERTEXBUFFER);

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
		mtrlDesc.renderTarget = ds::res::find("RT1", ResourceType::RENDERTARGET);
		_material = ds::res::createMaterial("ScreenShakeMaterial", mtrlDesc);

	}

	ScreenShakePostProcess::~ScreenShakePostProcess() {
	}

	void ScreenShakePostProcess::render() {
		ZoneTracker("ScreenShakePostProcess::render");
		unsigned int stride = sizeof(PTCVertex);
		unsigned int offset = 0;
		graphics::turnOffZBuffer();
		graphics::setVertexBuffer(_vertexBuffer, &stride, &offset, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		graphics::setMaterial(_material);

		//_cbData.x = _timer;// math::clamp(_timer / _descriptor.ttl, 0.0f, 1.0f);
		_constantBuffer.period = 0.25f;
		_constantBuffer.shakeAmount = 6.0f / 1024.0f;
		_constantBuffer.frequency = 5.0f;
		graphics::updateConstantBuffer(_cbID, &_constantBuffer, sizeof(ScreenShakeConstantBuffer));
		graphics::setPixelShaderConstantBuffer(_cbID);
		graphics::draw(6);
		graphics::turnOnZBuffer();
	}

	void ScreenShakePostProcess::tick(float dt) {
		if (_active) {
			_constantBuffer.timer += dt;

		}
	}

}