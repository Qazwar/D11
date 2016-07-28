#include "ScreenShakePostProcess.h"
#include "..\renderer\graphics.h"
#include "..\utils\Profiler.h"
#include "..\resources\ResourceContainer.h"
#include "..\shaders\postprocess\GrayFade_PS_Main.inc"
#include "..\shaders\postprocess\GrayFade_VS_Main.inc"

namespace ds {

	ScreenShakePostProcess::ScreenShakePostProcess(const ScreenShakePostProcessDescriptor& descriptor) : PostProcess() , _descriptor(descriptor) , _timer(0.0f) {
		assert(_descriptor.ttl > 0.0f);
		//_vertexBuffer = ds::res::find("PostProcessVertexBuffer",ResourceType::VERTEXBUFFER);
		_vertices[0] = PTCVertex(v3(-1, -1, 0), v2(0, 1), Color::WHITE);
		_vertices[1] = PTCVertex(v3(-1,  1, 0), v2(0, 0), Color::WHITE);
		_vertices[2] = PTCVertex(v3( 1,  1, 0), v2(1, 0), Color::WHITE);
		_vertices[3] = PTCVertex(v3( 1,  1, 0), v2(1, 0), Color::WHITE);
		_vertices[4] = PTCVertex(v3( 1, -1, 0), v2(1, 1), Color::WHITE);
		_vertices[5] = PTCVertex(v3(-1, -1, 0), v2(0, 1), Color::WHITE);

		ds::ConstantBufferDescriptor cbDesc;
		cbDesc.size = 16;
		_cbID = ds::res::createConstantBuffer("GrayFadeCB", cbDesc);

		RID ss_id = ds::res::find("SpriteSampler", ResourceType::SAMPLERSTATE);
		RID shader_id = ds::res::createEmptyShader("GrayFade");
		ds::Shader* s = ds::res::getShader(shader_id);
		graphics::getDevice()->CreateVertexShader(GrayFade_VS_Main, sizeof(GrayFade_VS_Main), 0, &s->vertexShader);
		graphics::getDevice()->CreatePixelShader(GrayFade_PS_Main, sizeof(GrayFade_PS_Main), 0, &s->pixelShader);
		s->samplerState = ds::res::getSamplerState(ss_id);

		// Position Texture Color layout
		ds::InputLayoutDescriptor ilDesc;
		ilDesc.num = 0;
		ilDesc.indices[ilDesc.num++] = 0;
		ilDesc.indices[ilDesc.num++] = 2;
		ilDesc.indices[ilDesc.num++] = 1;
		ilDesc.shader = INVALID_RID;
		ilDesc.byteCode = GrayFade_VS_Main;
		ilDesc.byteCodeSize = sizeof(GrayFade_VS_Main);

		RID il_id = ds::res::createInputLayout("GrayFadeLayout", ilDesc);

		ds::VertexBufferDescriptor vbDesc;
		vbDesc.dynamic = true;
		vbDesc.layout = il_id;
		vbDesc.size = 6;
		_vertexBuffer = ds::res::createVertexBuffer("GrayFadeVertexBuffer", vbDesc);

		ds::MaterialDescriptor mtrlDesc;
		mtrlDesc.shader = shader_id;
		mtrlDesc.blendstate = ds::res::find("DefaultBlendState",ResourceType::BLENDSTATE);
		mtrlDesc.texture = INVALID_RID;
		mtrlDesc.renderTarget = ds::res::find("RT1", ResourceType::RENDERTARGET);
		_material = ds::res::createMaterial("GrayFadeMaterial", mtrlDesc);
	}


	ScreenShakePostProcess::~ScreenShakePostProcess() {
	}

	void ScreenShakePostProcess::render() {
		ZoneTracker("GrayFadePostProcess::render");
		unsigned int stride = sizeof(PTCVertex);
		unsigned int offset = 0;
		graphics::turnOffZBuffer();
		graphics::setVertexBuffer(_vertexBuffer, &stride, &offset, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		graphics::setMaterial(_material);
		graphics::mapData(_vertexBuffer, _vertices, 6 * sizeof(PTCVertex));
		_cbData.x = math::clamp(_timer / _descriptor.ttl,0.0f,1.0f);
		graphics::updateConstantBuffer(_cbID, &_cbData, sizeof(v4));
		graphics::setPixelShaderConstantBuffer(_cbID);
		graphics::draw(6);
		graphics::turnOnZBuffer();
	}

	void ScreenShakePostProcess::tick(float dt) {
		if (_active) {
			_timer += dt;

		}
	}

}