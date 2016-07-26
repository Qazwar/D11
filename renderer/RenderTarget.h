#pragma once
#include "..\resources\ResourceDescriptors.h"

namespace ds {

	class RenderTarget {

	public:
		RenderTarget(const RenderTargetDescriptor& descriptor);
		~RenderTarget();
		bool init(ID3D11Device* device, int textureWidth, int textureHeight);		
		void begin(ID3D11DeviceContext* device);
		ID3D11ShaderResourceView* getShaderResourceView() const {
			return _shaderResourceView;
		}
	private:
		RenderTargetDescriptor _descriptor;
		ID3D11Texture2D* _renderTargetTexture;
		ID3D11RenderTargetView* _renderTargetView;
		ID3D11ShaderResourceView* _shaderResourceView;
		ID3D11Texture2D* _depthTexture;
		ID3D11DepthStencilView* _depthStencilView;
	};

}