#pragma once

class DrawCall {

};

class BindShaderCall : public DrawCall {

};


class IndexedDrawCall : public DrawCall {

public:
	int indexBuffer;
	int vertexBuffer;
	int layout;

};

namespace renderQueue {

	BindShaderCall* createBindShaderCall();

	IndexedDrawCall* createIndexedDrawCall();

	void submit(DrawCall* call);
}

