#pragma once
#include <stdint.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <DxErr.h>
#include <xnamath.h>
class Demo {

public:
	Demo();
	~Demo();
	bool initialize();
	void render();
	void shutdown();
private:
	ID3D11VertexShader* _solidColorVS;
	ID3D11PixelShader* _solidColorPS;

	ID3D11InputLayout* _inputLayout;
	ID3D11Buffer* _vertexBuffer;
	int _indexBuffer;

	ID3D11ShaderResourceView* _colorMap;
	ID3D11SamplerState* _colorMapSampler;
	int _alphaBlendState;

	ID3D11Buffer* _mvpCB;
	XMMATRIX _vpMatrix;

	XMFLOAT2 _positions[2];
};

