#pragma once
#include <stdint.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <DxErr.h>
#include "..\base\Settings.h"
#include "core\math\math_types.h"
#include "..\resources\ResourceDescriptors.h"
#include "core\Common.h"
#include "Bitmapfont.h"
#include "Camera.h"
#include "Viewport.h"

struct Access{

	enum ENUM {
		Read,
		Write,
		ReadWrite,
		Count
	};
};


namespace ds {

	class ResourceContainer;

	class SpriteBuffer;

	class SquareBuffer;
}

namespace graphics {

	enum BlendState {
		BS_NORMAL,
		BS_ONE,
		BS_ADDITIVE
	};
	
	bool initialize(HINSTANCE hInstance, HWND hwnd, const ds::Settings& settings);

	HWND getWindowsHandle();

	void createBlendStates();

	void createInternalSpriteBuffer();

	void createPostProcessResources();

	ds::SquareBuffer* createSquareBuffer(const char* name, int size, RID texture);

	ID3D11DeviceContext* getContext();

	ID3D11Device* getDevice();

	ID3D11DepthStencilView* getDepthStencilView();

	ds::OrthoCamera* getOrthoCamera();

	ds::FPSCamera* getFPSCamera();

	void setCamera(ds::Camera* camera);

	ds::Camera* getCamera();

	v2 getScreenCenter();

	float getScreenWidth();

	float getScreenHeight();

	void updateConstantBuffer(RID rid, void* data, size_t size);

	const ds::mat4& getViewProjectionMaxtrix();

	void beginRendering();

	void setIndexBuffer(RID rid);

	void setVertexBuffer(RID rid, uint32_t* stride, uint32_t* offset, D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	void mapData(RID rid, void* data, uint32_t size);

	void setShader(RID rid);

	void setClearColor(const ds::Color& clr);

	//void setInputLayout(RID rid);

	void setMaterial(RID rid);

	void setPixelShaderResourceView(RID rid, uint32_t slot = 0);

	void setVertexShaderConstantBuffer(RID rid);

	void setPixelShaderConstantBuffer(RID rid);

	void setGeometryShaderConstantBuffer(RID rid);

	void setBlendState(RID rid);

	void drawIndexed(uint32_t num);

	void draw(uint32_t num);

	void endRendering();

	void shutdown();

	void turnOnZBuffer();

	void turnOffZBuffer();

	ds::Ray getCameraRay(ds::Camera* camera);

	void updateSpriteConstantBuffer(ds::SpriteBufferCB& buffer);

	ds::SpriteBuffer* getSpriteBuffer();

	void setRenderTarget(RID rtID);

	void restoreBackbuffer();

	int addViewport(const ds::Viewport& vp);

	void setViewportPosition(int idx, const v2& pos);

	const ds::Viewport& getViewport(int idx);

	void selectViewport(int idx);

	void selectBlendState(RID rid);

}

