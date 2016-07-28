#include "GrayFadePostProcess.h"
#include "..\renderer\graphics.h"
#include "..\utils\Profiler.h"
#include "..\resources\ResourceContainer.h"
#include "..\shaders\postprocess\GrayFade_PS_Main.inc"
#include "..\shaders\postprocess\GrayFade_VS_Main.inc"

namespace ds {

	GrayFadePostProcess::GrayFadePostProcess(const GrayFadePostProcessDescriptor& descriptor) : PostProcess() , _descriptor(descriptor) , _timer(0.0f) {
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

		_vertexBuffer = ds::res::find("PostProcessVertexBuffer", ds::ResourceType::VERTEXBUFFER);
		ds::MaterialDescriptor mtrlDesc;
		mtrlDesc.shader = shader_id;
		mtrlDesc.blendstate = ds::res::find("DefaultBlendState",ResourceType::BLENDSTATE);
		mtrlDesc.texture = INVALID_RID;
		mtrlDesc.renderTarget = ds::res::find("RT1", ResourceType::RENDERTARGET);
		_material = ds::res::createMaterial("GrayFadeMaterial", mtrlDesc);
	}


	GrayFadePostProcess::~GrayFadePostProcess() {
	}

	void GrayFadePostProcess::render() {
		ZoneTracker("GrayFadePostProcess::render");
		unsigned int stride = sizeof(PTCVertex);
		unsigned int offset = 0;
		graphics::turnOffZBuffer();
		graphics::setVertexBuffer(_vertexBuffer, &stride, &offset, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		graphics::setMaterial(_material);
		_cbData.x = math::clamp(_timer / _descriptor.ttl,0.0f,1.0f);
		graphics::updateConstantBuffer(_cbID, &_cbData, sizeof(v4));
		graphics::setPixelShaderConstantBuffer(_cbID);
		graphics::draw(6);
		graphics::turnOnZBuffer();
	}

	void GrayFadePostProcess::tick(float dt) {
		if (_active) {
			_timer += dt;

		}
	}

}