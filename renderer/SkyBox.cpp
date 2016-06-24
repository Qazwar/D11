#include "SkyBox.h"
#include "..\renderer\graphics.h"
#include "..\math\math_types.h"
#include "..\math\matrix.h"
#include "..\resources\ResourceContainer.h"
#include "..\utils\Profiler.h"
#include "..\stats\DrawCounter.h"

// http://rbwhitaker.wikidot.com/skyboxes-1
// http://richardssoftware.net/Home/Post/25
// http://www.braynzarsoft.net/viewtutorial/q16390-20-cube-mapping-skybox
namespace ds {

	SkyBox::SkyBox(const SkyBoxDescriptor& descriptor) : _descriptor(descriptor) {

		
		v3 p[] = { v3(-1, -1, -1), v3(-1, -1, 1), v3(1, -1, 1), v3(1, -1, -1), v3(-1, 1, -1), v3(-1, 1, 1), v3(1, 1, 1), v3(1, 1, -1) };
		for (int i = 0; i < 8; ++i) {
			_vertices[i] = PNTCVertex(p[i] * _descriptor.scale, v2(0, 0), Color::WHITE);
		}
		
		//bottom face
		_cubeIndices[0] = 0;
		_cubeIndices[1] = 2;
		_cubeIndices[2] = 3;
		_cubeIndices[3] = 0;
		_cubeIndices[4] = 1;
		_cubeIndices[5] = 2;

		//top face
		_cubeIndices[6] = 4;
		_cubeIndices[7] = 6;
		_cubeIndices[8] = 5;
		_cubeIndices[9] = 4;
		_cubeIndices[10] = 7;
		_cubeIndices[11] = 6;

		//front face
		_cubeIndices[12] = 5;
		_cubeIndices[13] = 2;
		_cubeIndices[14] = 1;
		_cubeIndices[15] = 5;
		_cubeIndices[16] = 6;
		_cubeIndices[17] = 2;

		//back face
		_cubeIndices[18] = 0;
		_cubeIndices[19] = 7;
		_cubeIndices[20] = 4;
		_cubeIndices[21] = 0;
		_cubeIndices[22] = 3;
		_cubeIndices[23] = 7;

		//left face
		_cubeIndices[24] = 0;
		_cubeIndices[25] = 4;
		_cubeIndices[26] = 1;
		_cubeIndices[27] = 1;
		_cubeIndices[28] = 4;
		_cubeIndices[29] = 5;

		//right face
		_cubeIndices[30] = 2;
		_cubeIndices[31] = 6;
		_cubeIndices[32] = 3;
		_cubeIndices[33] = 3;
		_cubeIndices[34] = 6;
		_cubeIndices[35] = 7;

	}


	SkyBox::~SkyBox() {
	}

	void SkyBox::render() {
		ZoneTracker("SkyBox::render");

		mat4 world = matrix::m4identity();
		unsigned int stride = sizeof(PNTCVertex);
		unsigned int offset = 0;

		//graphics::setInputLayout(_descriptor.inputlayout);
		graphics::setVertexBuffer(_descriptor.vertexBuffer, &stride, &offset);

		graphics::mapData(_descriptor.indexBuffer, _cubeIndices, 36 * sizeof(int));

		graphics::setIndexBuffer(_descriptor.indexBuffer);
		graphics::setBlendState(_descriptor.blendstate);

		graphics::setShader(_descriptor.shader);
		graphics::setPixelShaderResourceView(_descriptor.colormap);

		Camera* camera = graphics::getCamera();
		assert(camera != 0);
		ds::mat4 mvp = world * camera->getViewProjectionMatrix();
		_buffer.viewProjectionMatrix = ds::matrix::mat4Transpose(mvp);
		_buffer.worldMatrix = ds::matrix::mat4Transpose(world);
		_buffer.cameraPos = camera->getPosition();
		_buffer.lightPos = v3(0,0,1);
		_buffer.diffuseColor = _diffuseColor;
		//graphics::mapData(_descriptor.vertexBuffer, _vertices.data(), _vertices.size() * sizeof(PNTCVertex));
		graphics::mapData(_descriptor.vertexBuffer, _vertices, 8 * sizeof(PNTCVertex));

		graphics::updateConstantBuffer(_descriptor.constantBuffer, &_buffer, sizeof(PNTCConstantBuffer));
		graphics::setVertexShaderConstantBuffer(_descriptor.constantBuffer);
		graphics::drawIndexed(36);
		++gDrawCounter->flushes;
		gDrawCounter->vertices += 8;
	}

}
