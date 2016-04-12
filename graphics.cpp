#include "graphics.h"
#include <D3Dcompiler.h>
#include <assert.h>
#include "utils\Log.h"
#include <vector>


namespace graphics {

	struct InputElementDescriptor {

		const char* semantic;
		uint32_t semanticIndex;
		DXGI_FORMAT format;
		uint32_t size;

		InputElementDescriptor(const char* sem, uint32_t index, DXGI_FORMAT f, uint32_t s) :
			semantic(sem), semanticIndex(index), format(f), size(s) {
		}
	};

	static const InputElementDescriptor INPUT_ELEMENT_DESCRIPTIONS[] = {

		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    12 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 12 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,        8 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT,        8 },
		{ "TEXCOORD", 2, DXGI_FORMAT_R32G32_FLOAT,        8 },
		{ "TEXCOORD", 3, DXGI_FORMAT_R32G32_FLOAT,        8 },
	};

	__declspec(align(16)) struct GraphicContext {
		HINSTANCE hInstance;
		HWND hwnd;

		D3D_DRIVER_TYPE driverType;
		D3D_FEATURE_LEVEL featureLevel;

		ID3D11Device* d3dDevice;
		ID3D11DeviceContext* d3dContext;
		IDXGISwapChain* swapChain;
		ID3D11RenderTargetView* backBufferTarget;

		XMMATRIX viewMatrix;
		XMMATRIX worldMatrix;
		XMMATRIX projectionMaxtrix;
		XMMATRIX viewProjectionMaxtrix;

		std::vector<ID3D11Buffer*> indexBuffers;
		std::vector<ID3D11Buffer*> vertexBuffers;
		std::vector<ID3D11Buffer*> constantBuffers;
		std::vector<ID3D11BlendState*> blendStates;
		std::vector<ID3D11InputLayout*> layouts;
		std::vector<Shader*> shaders;
		std::vector<ID3D11ShaderResourceView*> shaderResourceViews;
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


	bool initialize(HINSTANCE hInstance, HWND hwnd) {
		_context = new GraphicContext;
		_context->hInstance = hInstance;
		_context->hwnd = hwnd;

		RECT dimensions;
		GetClientRect(hwnd, &dimensions);

		unsigned int width = dimensions.right - dimensions.left;
		unsigned int height = dimensions.bottom - dimensions.top;

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
		swapChainDesc.BufferDesc.Width = width;
		swapChainDesc.BufferDesc.Height = height;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		DXGI_RATIONAL refreshRate;
		QueryRefreshRate(width, height, true,&refreshRate);
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
		viewport.Width = static_cast<float>(width);
		viewport.Height = static_cast<float>(height);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;

		_context->d3dContext->RSSetViewports(1, &viewport);

		_context->viewMatrix = XMMatrixIdentity();
		//D3DXMatrixOrthoLH(&m_orthoMatrix, (float)screenWidth, (float)screenHeight, screenNear, screenDepth);
		_context->projectionMaxtrix = XMMatrixOrthographicLH(static_cast<float>(width), static_cast<float>(height), 0.1f, 100.0f);
		//_context->projectionMaxtrix = XMMatrixOrthographicOffCenterLH(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height), 0.1f, 100.0f);
		_context->viewProjectionMaxtrix = XMMatrixMultiply(_context->viewMatrix, _context->projectionMaxtrix);

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

			for (size_t i = 0; i < _context->indexBuffers.size(); ++i) {
				_context->indexBuffers[i]->Release();
			}
			for (size_t i = 0; i < _context->blendStates.size(); ++i) {
				_context->blendStates[i]->Release();
			}
			for (size_t i = 0; i < _context->layouts.size(); ++i) {
				_context->layouts[i]->Release();
			}
			for (size_t i = 0; i < _context->shaders.size(); ++i) {
				Shader* shader = _context->shaders[i];
				if (shader->vertexShader != 0) {
					shader->vertexShader->Release();
				}
				if (shader->pixelShader != 0) {
					shader->pixelShader->Release();
				}
				if (shader->vertexShaderBuffer != 0) {
					shader->vertexShaderBuffer->Release();
				}
				if (shader->samplerState != 0) {
					shader->samplerState->Release();
				}
				delete _context->shaders[i];
			}
			for (size_t i = 0; i < _context->shaderResourceViews.size(); ++i) {
				_context->shaderResourceViews[i]->Release();
			}
			for (size_t i = 0; i < _context->vertexBuffers.size(); ++i) {
				_context->vertexBuffers[i]->Release();
			}
			for (size_t i = 0; i < _context->constantBuffers.size(); ++i) {
				_context->constantBuffers[i]->Release();
			}
			_context->backBufferTarget = 0;
			_context->swapChain = 0;
			_context->d3dContext = 0;
			_context->d3dDevice = 0;
		}
	}

	int compileShader(char* fileName) {
		Shader* s = new Shader;
		int idx = _context->shaders.size();
		s->vertexShaderBuffer = 0;
		bool compileResult = compileShader(fileName, "VS_Main", "vs_4_0", &s->vertexShaderBuffer);
		if (!compileResult)	{
			DXTRACE_MSG("Error compiling the vertex shader!");
			return -1;
		}
		HRESULT d3dResult;

		if (!createVertexShader(s->vertexShaderBuffer, &s->vertexShader)) {
			DXTRACE_MSG("Error creating the vertex shader!");
			return -1;
		}
		ID3DBlob* psBuffer = 0;
		compileResult = compileShader(fileName, "PS_Main", "ps_4_0", &psBuffer);
		if (!compileResult)	{
			DXTRACE_MSG("Error compiling pixel shader!");
			return -1;
		}

		if (!createPixelShader(psBuffer, &s->pixelShader)) {
			DXTRACE_MSG("Error creating pixel shader!");
			return -1;
		}
		psBuffer->Release();
		D3D11_SAMPLER_DESC colorMapDesc;
		ZeroMemory(&colorMapDesc, sizeof(colorMapDesc));
		colorMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		colorMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		colorMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;

		d3dResult = _context->d3dDevice->CreateSamplerState(&colorMapDesc, &s->samplerState);
		if (FAILED(d3dResult)) {
			DXTRACE_MSG("Failed to create SamplerState!");
			return -1;
		}
		_context->shaders.push_back(s);
		return idx;
	}

	bool compileShader(char* filePath, char* entry, char* shaderModel, ID3DBlob** buffer) {
		DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined( DEBUG ) || defined( _DEBUG )
		shaderFlags |= D3DCOMPILE_DEBUG;
#endif

		ID3DBlob* errorBuffer = 0;
		HRESULT result;

		result = D3DX11CompileFromFile(filePath, 0, 0, entry, shaderModel,shaderFlags, 0, 0, buffer, &errorBuffer, 0);

		if (FAILED(result))	{
			if (errorBuffer != 0) {
				OutputDebugStringA((char*)errorBuffer->GetBufferPointer());
				errorBuffer->Release();
			}
			return false;
		}
		if (errorBuffer != 0) {
			errorBuffer->Release();
		}
		return true;
	}

	ID3D11DeviceContext* getContext() {
		return _context->d3dContext;
	}

	bool createVertexShader(ID3DBlob* buffer, ID3D11VertexShader** shader) {
		HRESULT d3dResult = _context->d3dDevice->CreateVertexShader(buffer->GetBufferPointer(),buffer->GetBufferSize(), 0, shader);
		if (d3dResult < 0) {
			if (buffer) {
				buffer->Release();
			}
			return false;
		}
		return true;
	}

	bool createPixelShader(ID3DBlob* buffer, ID3D11PixelShader** shader) {
		HRESULT d3dResult = _context->d3dDevice->CreatePixelShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), 0, shader);
		if (d3dResult < 0) {
			if (buffer) {
				buffer->Release();
			}
			return false;
		}
		return true;
	}
	
	// ------------------------------------------------------
	// create input layout
	// ------------------------------------------------------
	int createInputLayout(int shaderIndex, Attribute* attribute) {		
		Attribute* ptr = attribute;
		uint32_t total = 0;
		while (*ptr != Attribute::End) {
			++total;
			++ptr;
		}
		D3D11_INPUT_ELEMENT_DESC* descriptors = new D3D11_INPUT_ELEMENT_DESC[total];
		uint32_t index = 0;
		uint32_t counter = 0;
		ptr = attribute;
		while ( *ptr != Attribute::End) {
			const InputElementDescriptor& d = INPUT_ELEMENT_DESCRIPTIONS[*ptr];
			D3D11_INPUT_ELEMENT_DESC& descriptor = descriptors[counter++];
			descriptor.SemanticName = d.semantic;
			descriptor.SemanticIndex = d.semanticIndex;
			descriptor.Format = d.format;
			descriptor.InputSlot = 0;
			descriptor.AlignedByteOffset = index;
			descriptor.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			descriptor.InstanceDataStepRate = 0;
			index += d.size;
			++ptr;
			assert(counter < 16);
		}
		int idx = _context->layouts.size();
		ID3D11InputLayout* layout = 0;
		Shader* s = _context->shaders[shaderIndex];
		HRESULT d3dResult = _context->d3dDevice->CreateInputLayout(descriptors, total, s->vertexShaderBuffer->GetBufferPointer(), s->vertexShaderBuffer->GetBufferSize(), &layout);
		if (d3dResult < 0) {
			return -1;
		}
		delete[] descriptors;
		_context->layouts.push_back(layout);
		return idx;
	}
	
	int loadTexture(const char* name) {
		int idx = _context->shaderResourceViews.size();
		ID3D11ShaderResourceView* srv = 0;
		HRESULT d3dResult = D3DX11CreateShaderResourceViewFromFile(_context->d3dDevice,name, 0, 0, &srv, 0);
		if (FAILED(d3dResult)) {
			DXTRACE_MSG("Failed to load the texture image!");
			return -1;
		}
		_context->shaderResourceViews.push_back(srv);
		return idx;
	}

	bool createSamplerState(ID3D11SamplerState** sampler) {
		D3D11_SAMPLER_DESC colorMapDesc;
		ZeroMemory(&colorMapDesc, sizeof(colorMapDesc));
		colorMapDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		colorMapDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		colorMapDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		colorMapDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		colorMapDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		colorMapDesc.MaxLOD = D3D11_FLOAT32_MAX;

		HRESULT d3dResult = _context->d3dDevice->CreateSamplerState(&colorMapDesc, sampler);
		if (FAILED(d3dResult)) {
			DXTRACE_MSG("Failed to create SamplerState!");
			return false;
		}
		return true;
	}

	// ------------------------------------------------------
	// create blend state
	// ------------------------------------------------------
	int createBlendState(D3D11_BLEND srcBlend, D3D11_BLEND destBlend) {
		int idx = _context->blendStates.size();
		D3D11_BLEND_DESC blendDesc;
		ZeroMemory(&blendDesc, sizeof(blendDesc));
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].BlendEnable = (srcBlend != D3D11_BLEND_ONE) ||(destBlend != D3D11_BLEND_ZERO);
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlend = srcBlend;
		blendDesc.RenderTarget[0].DestBlend = destBlend;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = srcBlend;
		blendDesc.RenderTarget[0].DestBlendAlpha = destBlend;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;
		
		ID3D11BlendState* state;
		HRESULT d3dResult = _context->d3dDevice->CreateBlendState(&blendDesc, &state);
		if (FAILED(d3dResult)) {
			DXTRACE_MSG("Failed to create blendstate!");
			return -1;
		}
		_context->blendStates.push_back(state);
		return idx;
	}

	// ------------------------------------------------------
	// create constant buffer
	// ------------------------------------------------------
	int createConstantBuffer(uint32_t size) {
		int index = _context->constantBuffers.size();
		D3D11_BUFFER_DESC constDesc;
		ZeroMemory(&constDesc, sizeof(constDesc));
		constDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constDesc.ByteWidth = size;
		constDesc.Usage = D3D11_USAGE_DEFAULT;
		ID3D11Buffer* buffer = 0;
		HRESULT d3dResult = _context->d3dDevice->CreateBuffer(&constDesc, 0, &buffer);
		if (FAILED(d3dResult))	{
			DXTRACE_MSG("Failed to create constant buffer!");
			return -1;
		}
		_context->constantBuffers.push_back(buffer);
		return index;
	}
	
	bool createBuffer(D3D11_BUFFER_DESC vertexDesc, D3D11_SUBRESOURCE_DATA resourceData, ID3D11Buffer** buffer) {
		HRESULT d3dResult = _context->d3dDevice->CreateBuffer(&vertexDesc, &resourceData, buffer);
		if (FAILED(d3dResult))	{
			DXTRACE_MSG("Failed to create vertex buffer!");
			return false;
		}
		return true;
	}

	bool createBuffer(D3D11_BUFFER_DESC bufferDesciption, ID3D11Buffer** buffer) {
		HRESULT d3dResult = _context->d3dDevice->CreateBuffer(&bufferDesciption, 0, buffer);
		if (FAILED(d3dResult))	{
			DXTRACE_MSG("Failed to create buffer!");
			return false;
		}
		return true;
	}

	int createBuffer(uint32_t bufferSize, D3D11_SUBRESOURCE_DATA resourceData) {
		D3D11_BUFFER_DESC bufferDesciption;
		ZeroMemory(&bufferDesciption, sizeof(bufferDesciption));
		bufferDesciption.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesciption.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesciption.ByteWidth = bufferSize;
		bufferDesciption.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		ID3D11Buffer* buffer = 0;
		HRESULT d3dResult = _context->d3dDevice->CreateBuffer(&bufferDesciption, &resourceData, &buffer);
		if (FAILED(d3dResult))	{
			DXTRACE_MSG("Failed to create buffer!");
			return -1;
		}
		int idx = _context->vertexBuffers.size();
		_context->vertexBuffers.push_back(buffer);
		return idx;
	}

	int createBuffer(uint32_t bufferSize) {
		D3D11_BUFFER_DESC bufferDesciption;
		ZeroMemory(&bufferDesciption, sizeof(bufferDesciption));
		bufferDesciption.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesciption.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesciption.ByteWidth = bufferSize;
		bufferDesciption.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		ID3D11Buffer* buffer = 0;
		HRESULT d3dResult = _context->d3dDevice->CreateBuffer(&bufferDesciption, 0, &buffer);
		if (FAILED(d3dResult))	{
			DXTRACE_MSG("Failed to create buffer!");
			return -1;
		}
		int idx = _context->vertexBuffers.size();
		_context->vertexBuffers.push_back(buffer);
		return idx;
	}

	int createQuadIndexBuffer(uint32_t numQuads) {
		int idx = _context->indexBuffers.size();
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = sizeof(uint32_t) * numQuads * 6;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;

		uint32_t* data = new uint32_t[numQuads * 6];
		int base = 0;
		int cnt = 0;
		for (int i = 0; i < numQuads; ++i) {
			data[base] = cnt;
			data[base + 1] = cnt + 1;
			data[base + 2] = cnt + 3;
			data[base + 3] = cnt + 1;
			data[base + 4] = cnt + 2;
			data[base + 5] = cnt + 3;
			base += 6;
			cnt += 4;
		}
		// Define the resource data.
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = data;
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;
		// Create the buffer with the device.
		ID3D11Buffer* buffer;
		HRESULT hr = _context->d3dDevice->CreateBuffer(&bufferDesc, &InitData, &buffer);
		if (FAILED(hr)) {
			delete[] data;
			DXTRACE_MSG("Failed to create index buffer!");
			return -1;
		}
		delete[] data;
		_context->indexBuffers.push_back(buffer);
		return idx;
	}

	int createIndexBuffer(uint32_t num,uint32_t* data) {
		int idx = _context->indexBuffers.size();
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = sizeof(uint32_t) * num;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;

		// Define the resource data.
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = data;
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;

		// Create the buffer with the device.
		ID3D11Buffer* buffer;
		HRESULT hr = _context->d3dDevice->CreateBuffer(&bufferDesc, &InitData, &buffer);
		if (FAILED(hr)) {
			DXTRACE_MSG("Failed to create index buffer!");
			return -1;
		}
		_context->indexBuffers.push_back(buffer);
		return idx;
	}

	const XMMATRIX& getViewProjectionMaxtrix() {
		return _context->viewProjectionMaxtrix;
	}

	// ------------------------------------------------------
	// begin rendering
	// ------------------------------------------------------
	void beginRendering() {
		float clearColor[4] = { 0.0f, 0.0f, 0.25f, 1.0f };
		_context->d3dContext->ClearRenderTargetView(_context->backBufferTarget, clearColor);
	}

	// ------------------------------------------------------
	// set index buffer
	// ------------------------------------------------------
	void setIndexBuffer(int index) {
		_context->d3dContext->IASetIndexBuffer(_context->indexBuffers[index], DXGI_FORMAT_R32_UINT, 0);
	}

	// ------------------------------------------------------
	// set blend state
	// ------------------------------------------------------
	void setBlendState(int index) {
		float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		_context->d3dContext->OMSetBlendState(_context->blendStates[index], blendFactor, 0xFFFFFFFF);
	}

	// ------------------------------------------------------
	// update constant buffer
	// ------------------------------------------------------
	void updateConstantBuffer(int index, void* data) {
		ID3D11Buffer* buffer = _context->constantBuffers[index];
		_context->d3dContext->UpdateSubresource(buffer, 0, 0, data, 0, 0);
	}

	// ------------------------------------------------------
	// set shader
	// ------------------------------------------------------
	void setShader(int shaderIndex) {
		Shader* s = _context->shaders[shaderIndex];
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
	void mapData(int bufferIndex, void* data, uint32_t size) {
		ID3D11Buffer* buffer = _context->vertexBuffers[bufferIndex];
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
	void setInputLayout(int layoutIndex) {
		ID3D11InputLayout* layout = _context->layouts[layoutIndex];
		_context->d3dContext->IASetInputLayout(layout);
	}

	void setPixelShaderResourceView(int index, uint32_t slot) {
		ID3D11ShaderResourceView* srv = _context->shaderResourceViews[index];
		_context->d3dContext->PSSetShaderResources(slot, 1, &srv);
	}

	void setVertexBuffer(int index, uint32_t* stride, uint32_t* offset) {
		_context->d3dContext->IASetVertexBuffers(0, 1, &_context->vertexBuffers[index], stride, offset);
		_context->d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void setVertexShaderConstantBuffer(int bufferIndex) {
		ID3D11Buffer* buffer = _context->constantBuffers[bufferIndex];
		_context->d3dContext->VSSetConstantBuffers(0, 1, &buffer);
	}

	void drawIndexed(int num) {
		_context->d3dContext->DrawIndexed(num * 6, 0, 0);
	}

	// ------------------------------------------------------
	// end rendering
	// ------------------------------------------------------
	void endRendering() {
		_context->swapChain->Present(0, 0);
	}

}