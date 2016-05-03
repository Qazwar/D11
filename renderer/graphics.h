#pragma once
#include <stdint.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <DxErr.h>
#include "..\base\Settings.h"
#include "..\math\math_types.h"
#include "..\resources\ResourceDescriptors.h"
#include "..\Common.h"
#include "Bitmapfont.h"
#include "Camera.h"

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
}

namespace graphics {
	
	bool initialize(HINSTANCE hInstance, HWND hwnd, const ds::Settings& settings);

	ID3D11DeviceContext* getContext();

	ID3D11Device* getDevice();

	void setCamera(ds::Camera* camera);

	ds::Camera* getCamera();

	v2 getScreenCenter();

	float getScreenWidth();

	float getScreenHeight();

	bool getMousePosition(v2* ret);

	void updateConstantBuffer(RID rid, void* data);

	const ds::mat4& getViewProjectionMaxtrix();

	void beginRendering(const ds::Color& color);

	void setIndexBuffer(RID rid);

	void setVertexBuffer(RID rid, uint32_t* stride, uint32_t* offset);

	void mapData(RID rid, void* data, uint32_t size);

	void setShader(RID rid);

	void setInputLayout(RID rid);

	void setPixelShaderResourceView(RID rid, uint32_t slot = 0);

	void setVertexShaderConstantBuffer(RID rid);

	void setBlendState(RID rid);

	void drawIndexed(uint32_t num);

	void endRendering();

	void shutdown();
}

