#pragma once
#include <stdint.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <DxErr.h>
#include <xnamath.h>
#include <Vector.h>

struct VertexPos {
	v3 pos;
	v2 tex0;

	VertexPos() {}

	VertexPos(const v3& p, const v2& t) : pos(p), tex0(t) {}
};


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
	//ID3D11Buffer* _vertexBuffer;
	int _vertexBuffer;
	int _indexBuffer;

	int _colorMap;
	int _alphaBlendState;

	VertexPos _vertices[16 * 4];

	ID3D11Buffer* _mvpCB;

	v2 _positions[2];
};

