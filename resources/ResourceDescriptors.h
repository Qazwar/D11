#pragma once
#include <stdint.h>

struct IndexBufferDescriptor {
	uint16_t id;
	uint32_t size;
	bool dynamic;
};

struct QuadIndexBufferDescriptor {

	uint16_t id;
	uint32_t size;

	QuadIndexBufferDescriptor() : id(0), size(0) {}
	QuadIndexBufferDescriptor(uint16_t _id, uint32_t _size) : id(_id), size(_size) {}

};

struct ConstantBufferDescriptor {

	uint16_t id;
	uint32_t size;

	ConstantBufferDescriptor() : id(0), size(0) {}
	ConstantBufferDescriptor(uint16_t _id, uint32_t _size) : id(_id), size(_size) {}
};

struct VertexBufferDescriptor {
	uint16_t id;
	uint32_t size;
	bool dynamic;
	uint32_t layout;
};

/*
shader {
id : 3
name : "SpriteShader"
file : "TextureMap.fx"
vertex_shader : "VS_Main"
pixel_shader : "PS_Main"
shader_model : "4_0"
}
*/
struct ShaderDescriptor {
	uint16_t id;
	const char* vertexShader;
	const char* pixelShader;
	const char* model;
	const char* file;
};
/*
input_layout {
id : 0
attributes : "Position3,Texcoord,Color0"
}
shader_resource_view {
id : 0
texture : "TextureArray.png"
}
blend_state {
id : 0
alpha_enabled : true
src_blend : "SRC_ALPHA"
dest_blend : "INV_SRC_ALPHA"
}
shader {
id : 0
file : "TextureMap.fx"
vertex_shader : "vss"
pixel_shader : "pss"
}
*/