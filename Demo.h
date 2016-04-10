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

	int _shaderIndex;
	int _inputLayout;
	ID3D11Buffer* _vertexBuffer;
	int _indexBuffer;

	int _colorMap;
	int _alphaBlendState;

	ID3D11Buffer* _mvpCB;

	XMFLOAT2 _positions[2];
};

