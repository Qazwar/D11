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
	}

	bool RenderTarget::init(ID3D11Device* device, int textureWidth, int textureHeight) {
			D3D11_TEXTURE2D_DESC textureDesc;
			HRESULT result;
			D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
			D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;


			// Initialize the render target texture description.
			ZeroMemory(&textureDesc, sizeof(textureDesc));

			// Setup the render target texture description.
			textureDesc.Width = textureWidth;
			textureDesc.Height = textureHeight;
			textureDesc.MipLevels = 1;
			textureDesc.ArraySize = 1;
			textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			textureDesc.SampleDesc.Count = 1;
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

			return true;
	}

	void RenderTarget::begin(ID3D11DeviceContext* device) {
		device->OMSetRenderTargets(1, &_renderTargetView, graphics::getDepthStencilView());
		// Clear the back buffer.
		device->ClearRenderTargetView(_renderTargetView, _descriptor.clearColor);
		// Clear the depth buffer.
		device->ClearDepthStencilView(graphics::getDepthStencilView(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
}