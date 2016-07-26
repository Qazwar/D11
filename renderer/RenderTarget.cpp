#include "RenderTarget.h"
#include "graphics.h"

namespace ds {

	// http://www.rastertek.com/dx11tut22.html
	RenderTarget::RenderTarget(const RenderTargetDescriptor& descriptor) : _descriptor(descriptor) {
		_renderTargetTexture = 0;
		_renderTargetView = 0;
		_shaderResourceView = 0;
	}


	RenderTarget::~RenderTarget() {
		if (_renderTargetTexture != 0) {
			_renderTargetTexture->Release();
		}
		if (_renderTargetView != 0) {
			_renderTargetView->Release();
		}
		if (_shaderResourceView != 0) {
			_shaderResourceView->Release();
		}
		if (_depthTexture != 0) {
			_depthTexture->Release();
		}
		if (_depthStencilView != 0) {
			_depthStencilView->Release();
		}
	}

	bool RenderTarget::init(ID3D11Device* device, int textureWidth, int textureHeight) {
			
			HRESULT result;
			
			// Initialize the render target texture description.
			D3D11_TEXTURE2D_DESC textureDesc;
			ZeroMemory(&textureDesc, sizeof(textureDesc));

			// Setup the render target texture description.
			textureDesc.Width = textureWidth;
			textureDesc.Height = textureHeight;
			textureDesc.MipLevels = 1;
			textureDesc.ArraySize = 1;
			textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Usage = D3D11_USAGE_DEFAULT;
			textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			textureDesc.CPUAccessFlags = 0;
			textureDesc.MiscFlags = 0;

			// Create the render target texture.
			result = device->CreateTexture2D(&textureDesc, NULL, &_renderTargetTexture);
			if (FAILED(result))
			{
				return false;
			}

			// Setup the description of the render target view.
			D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
			renderTargetViewDesc.Format = textureDesc.Format;
			renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			renderTargetViewDesc.Texture2D.MipSlice = 0;

			// Create the render target view.
			result = device->CreateRenderTargetView(_renderTargetTexture, &renderTargetViewDesc, &_renderTargetView);
			if (FAILED(result))
			{
				return false;
			}

			// Setup the description of the shader resource view.
			D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
			shaderResourceViewDesc.Format = textureDesc.Format;
			shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
			shaderResourceViewDesc.Texture2D.MipLevels = 1;

			// Create the shader resource view.
			result = device->CreateShaderResourceView(_renderTargetTexture, &shaderResourceViewDesc, &_shaderResourceView);
			if (FAILED(result))
			{
				return false;
			}

			D3D11_TEXTURE2D_DESC depthTexDesc;
			ZeroMemory(&depthTexDesc, sizeof(depthTexDesc));
			depthTexDesc.Width = textureWidth;
			depthTexDesc.Height = textureHeight;
			depthTexDesc.MipLevels = 1;
			depthTexDesc.ArraySize = 1;
			depthTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			depthTexDesc.SampleDesc.Count = 1;
			depthTexDesc.SampleDesc.Quality = 0;
			depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
			depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			depthTexDesc.CPUAccessFlags = 0;
			depthTexDesc.MiscFlags = 0;

			result = device->CreateTexture2D(&depthTexDesc, 0, &_depthTexture);
			if (FAILED(result))	{
				DXTRACE_MSG("Failed to create the depth texture!");
				return false;
			}

			D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
			ZeroMemory(&descDSV, sizeof(descDSV));
			descDSV.Format = depthTexDesc.Format;
			descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
			descDSV.Texture2D.MipSlice = 0;

			result = device->CreateDepthStencilView(_depthTexture, &descDSV, &_depthStencilView);
			if (FAILED(result))	{
				DXTRACE_MSG("Failed to create the depth stencil view!");
				return false;
			}

			return true;
	}

	void RenderTarget::begin(ID3D11DeviceContext* device) {
		device->OMSetRenderTargets(1, &_renderTargetView, _depthStencilView);
		// Clear the back buffer.
		device->ClearRenderTargetView(_renderTargetView, _descriptor.clearColor);
		// Clear the depth buffer.
		device->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
}