#pragma once
#include "..\resources\ResourceDescriptors.h"

namespace ds {

	class RenderTarget {

	public:
		RenderTarget(const RenderTargetDescriptor& descriptor);
		~RenderTarget();
		bool init(ID3D11Device* device, int textureWidth, int textureHeight);		
		void begin(ID3D11DeviceContext* device);
	private:
		RenderTargetDescriptor _descriptor;
		ID3D11Texture2D* _renderTargetTexture;
		ID3D11RenderTargetView* _renderTargetView;
		ID3D11ShaderResourceView* _shaderResourceView;
	};

}