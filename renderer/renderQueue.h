#pragma once

enum DrawCallType {
	BindShader,
	IndexedDraw
};

class DrawCall {

public:
	virtual void render() = 0;
	virtual const DrawCallType& getType() const = 0;
};

class BindShaderCall : public DrawCall {

public:
	void render() {}
	const DrawCallType& getType() const {
		return DrawCallType::BindShader;
	}
};


class IndexedDrawCall : public DrawCall {

public:
	int indexBuffer;
	int vertexBuffer;
	int layout;

	void render() {}
	const DrawCallType& getType() const {
		return DrawCallType::IndexedDraw;
	}

};

namespace renderQueue {

	BindShaderCall* createBindShaderCall();

	IndexedDrawCall* createIndexedDrawCall();

	void submit(DrawCall* call);

	void render();
}

