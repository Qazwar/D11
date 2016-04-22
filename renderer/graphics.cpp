#include "graphics.h"

#include <assert.h>
#include "..\utils\Log.h"
#include <vector>
#include "..\math\matrix.h"
#include "..\resources\ResourceContainer.h"
#include "sprites.h"

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

		ds::mat4 viewMatrix;
		ds::mat4 worldMatrix;
		ds::mat4 projectionMatrix;
		ds::mat4 viewProjectionMatrix;

		uint16_t screenWidth;
		uint16_t screenHeight;

		v2 viewportCenter;
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
		swapChainDesc.SampleDesc.Count = 1;
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

		_context->d3dContext->OMSetRenderTargets(1, &_context->backBufferTarget, 0);

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

		_context->viewMatrix = ds::matrix::m4identity();
		_context->projectionMatrix = ds::matrix::mat4OrthoLH(static_cast<float>(settings.screenWidth), static_cast<float>(settings.screenHeight), 0.1f, 100.0f);
		_context->viewProjectionMatrix = _context->viewMatrix * _context->projectionMatrix;

		return true;
	}

	// ------------------------------------------------------
	// shutdown
	// ------------------------------------------------------
	void shutdown() {
		if (_context != 0) {
			if (_context->backBufferTarget) _context->backBufferTarget->Release();
			if (_context->swapChain) _context->swapChain->Release();
			if (_context->d3dContext) _context->d3dContext->Release();
			if (_context->d3dDevice) _context->d3dDevice->Release();
			delete _context;
		}
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
	// get mouse position
	// ------------------------------------------------------
	bool getMousePosition(v2* ret) {
		POINT p;
		if (GetCursorPos(&p)) {
			if (ScreenToClient(_context->hwnd, &p)) {
				ret->x = p.x;
				ret->y = _context->screenHeight - p.y;
				return true;
			}
		}
		return false;
	}

	// ------------------------------------------------------
	// begin rendering
	// ------------------------------------------------------
	void beginRendering(const ds::Color& color) {
		_context->d3dContext->ClearRenderTargetView(_context->backBufferTarget, color);
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
	void updateConstantBuffer(RID rid, void* data) {
		ID3D11Buffer* buffer = ds::res::getConstantBuffer(rid);
		_context->d3dContext->UpdateSubresource(buffer, 0, 0, data, 0, 0);
	}

	// ------------------------------------------------------
	// set shader
	// ------------------------------------------------------
	void setShader(RID rid) {
		ds::Shader* s = ds::res::getShader(rid);
		if (s->vertexShader != 0) {
			_context->d3dContext->VSSetShader(s->vertexShader, 0, 0);
		}
		if (s->pixelShader != 0) {
			_context->d3dContext->PSSetShader(s->pixelShader, 0, 0);
		}
		_context->d3dContext->PSSetSamplers(0, 1, &s->samplerState);
	}

	// ------------------------------------------------------
	// map data to vertex buffer
	// ------------------------------------------------------
	void mapData(RID rid, void* data, uint32_t size) {
		ID3D11Buffer* buffer = ds::res::getVertexBuffer(rid);
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

	void setVertexBuffer(RID rid, uint32_t* stride, uint32_t* offset) {
		ID3D11Buffer* buffer = ds::res::getVertexBuffer(rid);
		_context->d3dContext->IASetVertexBuffers(0, 1, &buffer, stride, offset);
		_context->d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void setVertexShaderConstantBuffer(RID rid) {
		ID3D11Buffer* buffer = ds::res::getConstantBuffer(rid);
		_context->d3dContext->VSSetConstantBuffers(0, 1, &buffer);
	}

	void drawIndexed(uint32_t num) {
		_context->d3dContext->DrawIndexed(num, 0, 0);
	}

	float getScreenWidth() {
		return _context->screenWidth;
	}

	float getScreenHeight() {
		return _context->screenHeight;
	}

	// ------------------------------------------------------
	// end rendering
	// ------------------------------------------------------
	void endRendering() {
		_context->swapChain->Present(0, 0);
	}

}