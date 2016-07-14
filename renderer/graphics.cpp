#include "graphics.h"

#include <assert.h>
#include "..\utils\Log.h"
#include <vector>
#include "..\math\matrix.h"
#include "..\resources\ResourceContainer.h"
#include "sprites.h"
#include "..\base\InputSystem.h"
#include "..\resources\Resource.h"
#include "..\shaders\Sprite_VS_Main.inc"
#include "..\shaders\Sprite_PS_Main.inc"
#include "..\shaders\Sprite_GS_Main.inc"

namespace graphics {

	struct GraphicContext {
		HINSTANCE hInstance;
		HWND hwnd;

		D3D_DRIVER_TYPE driverType;
		D3D_FEATURE_LEVEL featureLevel;

		ID3D11Device* d3dDevice;
		ID3D11DeviceContext* d3dContext;
		IDXGISwapChain* swapChain;
		ID3D11RenderTargetView* backBufferTarget;

		ID3D11Texture2D* depthTexture;
		ID3D11DepthStencilView* depthStencilView;

		ID3D11DepthStencilState* depthDisabledStencilState;
		ID3D11DepthStencilState* depthEnabledStencilState;

		ds::mat4 viewMatrix;
		ds::mat4 worldMatrix;
		ds::mat4 projectionMatrix;
		ds::mat4 viewProjectionMatrix;

		uint16_t screenWidth;
		uint16_t screenHeight;

		ds::Camera* camera;

		v2 viewportCenter;

		ds::OrthoCamera* orthoCamera;
		ds::FPSCamera* fpsCamera;

		ID3D11Buffer* spriteCB;
		ds::SpriteBuffer* sprites;
	};

	static GraphicContext* _context;

	// This function was inspired by:
	// http://www.rastertek.com/dx11tut03.html
	bool QueryRefreshRate(UINT screenWidth, UINT screenHeight, bool vsync, DXGI_RATIONAL* rational)	{
		DXGI_RATIONAL refreshRate = { 0, 1 };
		if (vsync)
		{
			IDXGIFactory* factory;
			IDXGIAdapter* adapter;
			IDXGIOutput* adapterOutput;
			DXGI_MODE_DESC* displayModeList;

			// Create a DirectX graphics interface factory.
			HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
			if (FAILED(hr))
			{
				MessageBox(0,
					TEXT("Could not create DXGIFactory instance."),
					TEXT("Query Refresh Rate"),
					MB_OK);

				return false;
			}

			hr = factory->EnumAdapters(0, &adapter);
			if (FAILED(hr))
			{
				MessageBox(0,
					TEXT("Failed to enumerate adapters."),
					TEXT("Query Refresh Rate"),
					MB_OK);

				return false;
			}

			hr = adapter->EnumOutputs(0, &adapterOutput);
			if (FAILED(hr))
			{
				MessageBox(0,
					TEXT("Failed to enumerate adapter outputs."),
					TEXT("Query Refresh Rate"),
					MB_OK);

				return false;
			}

			UINT numDisplayModes;
			hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, nullptr);
			if (FAILED(hr))
			{
				MessageBox(0,
					TEXT("Failed to query display mode list."),
					TEXT("Query Refresh Rate"),
					MB_OK);

				return false;
			}

			displayModeList = new DXGI_MODE_DESC[numDisplayModes];
			assert(displayModeList);

			hr = adapterOutput->GetDisplayModeList(DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numDisplayModes, displayModeList);
			if (FAILED(hr))
			{
				MessageBox(0,
					TEXT("Failed to query display mode list."),
					TEXT("Query Refresh Rate"),
					MB_OK);

				return false;
			}

			// Now store the refresh rate of the monitor that matches the width and height of the requested screen.
			for (UINT i = 0; i < numDisplayModes; ++i)
			{
				if (displayModeList[i].Width == screenWidth && displayModeList[i].Height == screenHeight)
				{
					refreshRate = displayModeList[i].RefreshRate;
				}
			}

			delete[] displayModeList;
			if (adapterOutput) {
				adapterOutput->Release();
			}
			if (adapter) {
				adapter->Release();
			}
			if (factory) {
				factory->Release();
			}
		}

		*rational = refreshRate;
		LOG << "refresh: " << refreshRate.Numerator << " " << refreshRate.Denominator;
		return true;
	}

	void createInternalSpriteBuffer() {
		// FIXME: create SpriteBufferCB
		D3D11_BUFFER_DESC constDesc;
		ZeroMemory(&constDesc, sizeof(constDesc));
		constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constDesc.ByteWidth = sizeof(ds::SpriteBufferCB);
		constDesc.Usage = D3D11_USAGE_DYNAMIC;
		constDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		HRESULT d3dResult = _context->d3dDevice->CreateBuffer(&constDesc, 0, &_context->spriteCB);
		if (FAILED(d3dResult)) {
			DXTRACE_MSG("Failed to create constant buffer!");
		}

		ds::SamplerStateDescriptor ssDesc;
		ssDesc.addressU = 2;
		ssDesc.addressV = 2;
		ssDesc.addressW = 2;
		RID ss_id = ds::res::createSamplerState("SpriteSampler", ssDesc);

		RID shader_id = ds::res::createEmptyShader("SpriteShader");
		ds::Shader* s = ds::res::getShader(shader_id);

		_context->d3dDevice->CreateVertexShader(Sprite_VS_Main, sizeof(Sprite_VS_Main), 0, &s->vertexShader);
		_context->d3dDevice->CreatePixelShader(Sprite_PS_Main, sizeof(Sprite_PS_Main), 0, &s->pixelShader);
		_context->d3dDevice->CreateGeometryShader(Sprite_GS_Main, sizeof(Sprite_GS_Main), 0, &s->geometryShader);
		s->samplerState = ds::res::getSamplerState(ss_id);
		/*
		ds::ShaderDescriptor shaderDesc;
		shaderDesc.file = "content\\shaders\\Sprite.fx";
		shaderDesc.vertexShader = "VS_Main";
		shaderDesc.pixelShader = "PS_Main";
		shaderDesc.geometryShader = "GS_Main";
		shaderDesc.model = "4_0";
		shaderDesc.samplerState = ss_id;
		RID shader_id = ds::res::createShader("SpriteShader", shaderDesc);
		*/

		ds::InputLayoutDescriptor ilDesc;
		ilDesc.num = 0;
		ilDesc.indices[ilDesc.num++] = 0;
		ilDesc.indices[ilDesc.num++] = 1;
		ilDesc.indices[ilDesc.num++] = 3;
		ilDesc.indices[ilDesc.num++] = 1;
		//ilDesc.shader = shader_id;
		ilDesc.shader = INVALID_RID;
		ilDesc.byteCode = Sprite_VS_Main;
		ilDesc.byteCodeSize = sizeof(Sprite_VS_Main);

		RID il_id = ds::res::createInputLayout("SpriteInputLayout", ilDesc);

		ds::VertexBufferDescriptor vbDesc;
		vbDesc.dynamic = true;
		vbDesc.layout = il_id;
		vbDesc.size = 8192;
		RID vb_id = ds::res::createVertexBuffer("SpriteVertexBuffer", vbDesc);

		ds::BlendStateDescriptor bsDesc;
		bsDesc.alphaEnabled = true;
		bsDesc.srcBlend = ds::res::findBlendState("SRC_ALPHA");
		bsDesc.srcAlphaBlend = ds::res::findBlendState("SRC_ALPHA");
		bsDesc.destBlend = ds::res::findBlendState("INV_SRC_ALPHA");
		bsDesc.destAlphaBlend = ds::res::findBlendState("INV_SRC_ALPHA");
		RID bs_id = ds::res::createBlendState("SpriteBlendState", bsDesc);

		ds::MaterialDescriptor mtrlDesc;
		mtrlDesc.shader = shader_id;
		mtrlDesc.blendstate = bs_id;
		mtrlDesc.texture = INVALID_RID;
		RID mtrl_id = ds::res::createMaterial("SpriteMaterial", mtrlDesc);

		ds::SpriteBufferDescriptor spDesc;
		spDesc.size = 4096;
		spDesc.vertexBuffer = vb_id;
		spDesc.material = mtrl_id;
		_context->sprites = new ds::SpriteBuffer(spDesc);
	}

	bool initialize(HINSTANCE hInstance, HWND hwnd, const ds::Settings& settings) {
		_context = new GraphicContext;
		_context->hInstance = hInstance;
		_context->hwnd = hwnd;
		_context->screenWidth = settings.screenWidth;
		_context->screenHeight = settings.screenHeight;
		RECT dimensions;
		GetClientRect(hwnd, &dimensions);

		D3D_DRIVER_TYPE driverTypes[] = {
			D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP,
			D3D_DRIVER_TYPE_REFERENCE, D3D_DRIVER_TYPE_SOFTWARE
		};

		unsigned int totalDriverTypes = ARRAYSIZE(driverTypes);

		D3D_FEATURE_LEVEL featureLevels[] = {
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0
		};

		unsigned int totalFeatureLevels = ARRAYSIZE(featureLevels);

		

		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
		swapChainDesc.BufferCount = 1;
		swapChainDesc.BufferDesc.Width = settings.screenWidth;
		swapChainDesc.BufferDesc.Height = settings.screenHeight;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_RATIONAL refreshRate;
		QueryRefreshRate(settings.screenWidth, settings.screenHeight, true, &refreshRate);
		//swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		//swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferDesc.RefreshRate = refreshRate;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = hwnd;
		swapChainDesc.Windowed = true;
		swapChainDesc.SampleDesc.Count = 4;
		swapChainDesc.SampleDesc.Quality = 0;

		unsigned int creationFlags = 0;

#ifdef _DEBUG
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HRESULT result;
		unsigned int driver = 0;

		for (driver = 0; driver < totalDriverTypes; ++driver) {
			result = D3D11CreateDeviceAndSwapChain(0, driverTypes[driver], 0, creationFlags,
				featureLevels, totalFeatureLevels,
				D3D11_SDK_VERSION, &swapChainDesc, &_context->swapChain, &_context->d3dDevice, &_context->featureLevel, &_context->d3dContext);

			if (SUCCEEDED(result)) {
				_context->driverType = driverTypes[driver];
				break;
			}
		}

		if (FAILED(result))	{
			DXTRACE_MSG("Failed to create the Direct3D device!");
			return false;
		}

		ID3D11Texture2D* backBufferTexture;

		result = _context->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferTexture);

		if (FAILED(result))	{
			DXTRACE_MSG("Failed to get the swap chain back buffer!");
			return false;
		}

		result = _context->d3dDevice->CreateRenderTargetView(backBufferTexture, 0, &_context->backBufferTarget);

		if (backBufferTexture)
			backBufferTexture->Release();

		if (FAILED(result))
		{
			DXTRACE_MSG("Failed to create the render target view!");
			return false;
		}

		

		D3D11_TEXTURE2D_DESC depthTexDesc;
		ZeroMemory(&depthTexDesc, sizeof(depthTexDesc));
		depthTexDesc.Width = settings.screenWidth;
		depthTexDesc.Height = settings.screenHeight;
		depthTexDesc.MipLevels = 1;
		depthTexDesc.ArraySize = 1;
		depthTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthTexDesc.SampleDesc.Count = 4;
		depthTexDesc.SampleDesc.Quality = 0;
		depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
		depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthTexDesc.CPUAccessFlags = 0;
		depthTexDesc.MiscFlags = 0;

		result = _context->d3dDevice->CreateTexture2D(&depthTexDesc, 0, &_context->depthTexture);
		if (FAILED(result))	{
			DXTRACE_MSG("Failed to create the depth texture!");
			return false;
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory(&descDSV, sizeof(descDSV));
		descDSV.Format = depthTexDesc.Format;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
		descDSV.Texture2D.MipSlice = 0;

		result = _context->d3dDevice->CreateDepthStencilView(_context->depthTexture, &descDSV, &_context->depthStencilView);
		if (FAILED(result))	{
			DXTRACE_MSG("Failed to create the depth stencil view!");
			return false;
		}

		_context->d3dContext->OMSetRenderTargets(1, &_context->backBufferTarget, _context->depthStencilView);

		D3D11_VIEWPORT viewport;
		viewport.Width = static_cast<float>(settings.screenWidth);
		viewport.Height = static_cast<float>(settings.screenHeight);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;

		_context->d3dContext->RSSetViewports(1, &viewport);

		_context->viewportCenter.x = settings.screenWidth / 2;
		_context->viewportCenter.y = settings.screenHeight / 2;

		D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;
		ZeroMemory(&depthDisabledStencilDesc, sizeof(depthDisabledStencilDesc));

		// Now create a second depth stencil state which turns off the Z buffer for 2D rendering.  The only difference is 
		// that DepthEnable is set to false, all other parameters are the same as the other depth stencil state.
		depthDisabledStencilDesc.DepthEnable = false;
		depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
		depthDisabledStencilDesc.StencilEnable = true;
		depthDisabledStencilDesc.StencilReadMask = 0xFF;
		depthDisabledStencilDesc.StencilWriteMask = 0xFF;
		depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Create the state using the device.
		result = _context->d3dDevice->CreateDepthStencilState(&depthDisabledStencilDesc, &_context->depthDisabledStencilState);
		if (FAILED(result))	{
			return false;
		}

		depthDisabledStencilDesc.DepthEnable = true;
		result = _context->d3dDevice->CreateDepthStencilState(&depthDisabledStencilDesc, &_context->depthEnabledStencilState);
		if (FAILED(result))	{
			return false;
		}

		_context->viewMatrix = ds::matrix::m4identity();
		_context->projectionMatrix = ds::matrix::mat4OrthoLH(static_cast<float>(settings.screenWidth), static_cast<float>(settings.screenHeight), 0.1f, 100.0f);
		_context->viewProjectionMatrix = _context->viewMatrix * _context->projectionMatrix;

		_context->camera = 0;
		_context->orthoCamera = new ds::OrthoCamera(graphics::getScreenWidth(), graphics::getScreenHeight());
		_context->fpsCamera = new ds::FPSCamera(graphics::getScreenWidth(), graphics::getScreenHeight());
		
		return true;
	}

	// ------------------------------------------------------
	// shutdown
	// ------------------------------------------------------
	void shutdown() {
		if (_context != 0) {
			_context->spriteCB->Release();
			delete _context->sprites;
			delete _context->orthoCamera;
			delete _context->fpsCamera;
			if (_context->backBufferTarget) _context->backBufferTarget->Release();
			if (_context->swapChain) _context->swapChain->Release();
			if (_context->d3dContext) _context->d3dContext->Release();
			if (_context->depthStencilView) _context->depthStencilView->Release();
			if (_context->depthTexture) _context->depthTexture->Release();
			if (_context->depthDisabledStencilState) _context->depthDisabledStencilState->Release();
			if (_context->depthEnabledStencilState) _context->depthEnabledStencilState->Release();
			if (_context->d3dDevice) _context->d3dDevice->Release();		
			
			delete _context;
		}
	}

	void setCamera(ds::Camera* camera) {
		_context->camera = camera;
	}

	ds::OrthoCamera* getOrthoCamera() {
		return _context->orthoCamera;
	}

	ds::FPSCamera* getFPSCamera() {
		return _context->fpsCamera;
	}

	ds::Camera* getCamera() {
		return _context->camera;
	}

	ID3D11DeviceContext* getContext() {
		return _context->d3dContext;
	}

	ID3D11Device* getDevice() {
		return _context->d3dDevice;
	}

	// ------------------------------------------------------
	// get view projection matrix
	// ------------------------------------------------------
	const ds::mat4& getViewProjectionMaxtrix() {
		return _context->viewProjectionMatrix;
	}

	v2 getScreenCenter() {
		return _context->viewportCenter;
	}

	// ------------------------------------------------------
	// begin rendering
	// ------------------------------------------------------
	void beginRendering(const ds::Color& color) {
		_context->d3dContext->ClearRenderTargetView(_context->backBufferTarget, color);
		_context->d3dContext->ClearDepthStencilView(_context->depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0.0);
	}

	// ------------------------------------------------------
	// set index buffer
	// ------------------------------------------------------
	void setIndexBuffer(RID rid) {		
		_context->d3dContext->IASetIndexBuffer(ds::res::getIndexBuffer(rid), DXGI_FORMAT_R32_UINT, 0);
	}

	// ------------------------------------------------------
	// set blend state
	// ------------------------------------------------------
	void setBlendState(RID rid) {
		float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		_context->d3dContext->OMSetBlendState(ds::res::getBlendState(rid), blendFactor, 0xFFFFFFFF);
	}

	// ------------------------------------------------------
	// update constant buffer
	// ------------------------------------------------------
	void updateConstantBuffer(RID rid, void* data,size_t size) {
		ID3D11Buffer* buffer = ds::res::getConstantBuffer(rid);
		//_context->d3dContext->UpdateSubresource(buffer, 0, 0, data, 0, 0);
		D3D11_MAPPED_SUBRESOURCE resource;
		HRESULT hResult = _context->d3dContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
		void* ptr = resource.pData;
		// Copy the data into the vertex buffer.
		memcpy(ptr, data, size);
		_context->d3dContext->Unmap(buffer, 0);
	}

	void setMaterial(RID rid) {
		ds::Material* m = ds::res::getMaterial(rid);
		setBlendState(m->blendState);
		setShader(m->shader);
		if (m->texture != INVALID_RID) {
			setPixelShaderResourceView(m->texture);
		}
		else {
			// FIXME: set NULL as pixel shader resource
		}
	}
	// ------------------------------------------------------
	// set shader
	// ------------------------------------------------------
	void setShader(RID rid) {
		ds::Shader* s = ds::res::getShader(rid);
		if (s->vertexShader != 0) {
			_context->d3dContext->VSSetShader(s->vertexShader, 0, 0);
		}
		else {
			_context->d3dContext->VSSetShader(NULL, NULL, 0);
		}
		if (s->pixelShader != 0) {
			_context->d3dContext->PSSetShader(s->pixelShader, 0, 0);
		}
		else {
			_context->d3dContext->PSSetShader(NULL, NULL, 0);
		}
		if (s->geometryShader != 0) {
			_context->d3dContext->GSSetShader(s->geometryShader, 0, 0);
		}
		else {
			_context->d3dContext->GSSetShader(NULL, NULL, 0);
		}
		_context->d3dContext->PSSetSamplers(0, 1, &s->samplerState);
	}

	// ------------------------------------------------------
	// map data to vertex buffer
	// ------------------------------------------------------
	void mapData(RID rid, void* data, uint32_t size) {
		
		ID3D11Buffer* buffer = 0;
		if (ds::res::contains(rid, ds::ResourceType::VERTEXBUFFER)) {
			buffer = ds::res::getVertexBuffer(rid);
		}
		if (ds::res::contains(rid, ds::ResourceType::INDEXBUFFER)) {
			buffer = ds::res::getIndexBuffer(rid);
		}
		assert(buffer != 0);
		D3D11_MAPPED_SUBRESOURCE resource;
		HRESULT hResult = _context->d3dContext->Map(buffer, 0,D3D11_MAP_WRITE_DISCARD, 0, &resource);
		// This will be S_OK
		if (hResult == S_OK) {
			void* ptr = resource.pData;
			// Copy the data into the vertex buffer.
			memcpy(ptr, data, size);
			_context->d3dContext->Unmap(buffer, 0);
		}
		else {
			LOG << "ERROR mapping data";
		}
	}

	ds::SpriteBuffer* getSpriteBuffer() {
		return _context->sprites;
	}

	void updateSpriteConstantBuffer(const ds::SpriteBufferCB& buffer) {
		D3D11_MAPPED_SUBRESOURCE resource;
		HRESULT hResult = _context->d3dContext->Map(_context->spriteCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
		void* ptr = resource.pData;
		// Copy the data into the vertex buffer.
		memcpy(ptr, &buffer, sizeof(ds::SpriteBufferCB));
		_context->d3dContext->Unmap(_context->spriteCB, 0);
		_context->d3dContext->VSSetConstantBuffers(0, 1, &_context->spriteCB);
		_context->d3dContext->GSSetConstantBuffers(0, 1, &_context->spriteCB);
	}

	// ------------------------------------------------------
	// set input layout
	// ------------------------------------------------------
	void setInputLayout(RID rid) {
		ID3D11InputLayout* layout = ds::res::getInputLayout(rid);
		_context->d3dContext->IASetInputLayout(layout);
	}

	void setPixelShaderResourceView(RID rid, uint32_t slot) {
		ID3D11ShaderResourceView* srv = ds::res::getShaderResourceView(rid);
		_context->d3dContext->PSSetShaderResources(slot, 1, &srv);
	}

	void setVertexBuffer(RID rid, uint32_t* stride, uint32_t* offset, D3D11_PRIMITIVE_TOPOLOGY topology) {
		ds::VertexBufferResource* res = static_cast<ds::VertexBufferResource*>(ds::res::getResource(rid, ds::ResourceType::VERTEXBUFFER));
		ID3D11InputLayout* layout = ds::res::getInputLayout(res->getInputLayout());
		_context->d3dContext->IASetInputLayout(layout);
		ID3D11Buffer* buffer = res->get();
		_context->d3dContext->IASetVertexBuffers(0, 1, &buffer, stride, offset);
		_context->d3dContext->IASetPrimitiveTopology(topology);
	}

	void setVertexShaderConstantBuffer(RID rid) {
		ID3D11Buffer* buffer = ds::res::getConstantBuffer(rid);
		_context->d3dContext->VSSetConstantBuffers(0, 1, &buffer);
	}

	void setPixelShaderConstantBuffer(RID rid) {
		ID3D11Buffer* buffer = ds::res::getConstantBuffer(rid);
		_context->d3dContext->PSSetConstantBuffers(0, 1, &buffer);
	}

	void setGeometryShaderConstantBuffer(RID rid) {
		ID3D11Buffer* buffer = ds::res::getConstantBuffer(rid);
		_context->d3dContext->GSSetConstantBuffers(0, 1, &buffer);
	}

	void drawIndexed(uint32_t num) {
		_context->d3dContext->DrawIndexed(num, 0, 0);
	}

	void draw(uint32_t num) {
		_context->d3dContext->Draw(num, 0);
	}

	float getScreenWidth() {
		return _context->screenWidth;
	}

	float getScreenHeight() {
		return _context->screenHeight;
	}

	void turnOnZBuffer() {
		_context->d3dContext->OMSetDepthStencilState(_context->depthEnabledStencilState, 1);
	}

	void turnOffZBuffer() {
		_context->d3dContext->OMSetDepthStencilState(_context->depthDisabledStencilState, 1);
	}

	ds::Ray getCameraRay(ds::Camera* camera) {
		ds::Ray ray;
		v2 mp = ds::input::getMousePosition();
		ds::mat4 projection = camera->getProjectionMatrix();
		float px = (((2.0f * mp.x) / _context->screenWidth) - 1.0f) / projection._11;

		float py = (((2.0f * mp.y) / _context->screenHeight) - 1.0f) / projection._22;

		ray.origin = v3(0.0f, 0.0f, 0.0f);
		ray.direction = v3(px, py, 1.0f);

		ds::mat4 view = camera->getViewMatrix();
		view = ds::matrix::mat4Inverse(view);

		ray.origin = ds::matrix::transformCoordinate(ray.origin, view);
		ray.direction = ds::matrix::transformNormal(ray.direction, view);
		ray.direction = normalize(ray.direction);
		for (int i = 0; i < 3; ++i) {
			ray.invDir.data[i] = 1.0f / ray.direction.data[i];
			ray.sign[i] = (ray.invDir.data[i] < 0.0f);
		}
		return ray;
	}

	// ------------------------------------------------------
	// end rendering
	// ------------------------------------------------------
	void endRendering() {
		_context->swapChain->Present(0, 0);
	}

}