#pragma once
#include <stdint.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <DxErr.h>
#include <xnamath.h>
#include <Vector.h>
#include "renderer\sprites.h"

class Demo {

public:
	Demo();
	~Demo();
	bool initialize();
	void render();
	void shutdown();
private:
	SpriteBuffer* _spriteBuffer;
	v2 _positions[256];
	v2 _scales[256];
	float _rotations[256];
	int _num;
	//int _shaderIndex;
	//int _inputLayout;
	//ID3D11Buffer* _vertexBuffer;
	//int _vertexBuffer;
	//int _indexBuffer;
	//int _colorMap;
	//int _alphaBlendState;
	//VertexPos _vertices[16 * 4];
	//ID3D11Buffer* _mvpCB;
	//v2 _positions[2];
};

