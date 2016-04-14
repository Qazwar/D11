#include "BaseApp.h"
#include "..\graphics.h"
#include "..\utils\Log.h"
#include "..\io\json.h"
#include "..\utils\GlobalStringBuffer.h"
#include "..\io\FileRepository.h"
#include "..\utils\Profiler.h"

namespace ds {

	BaseApp::BaseApp() {
		_dt = 1.0f / 60.0f;
		_lastTime = GetTickCount();
		_delta = 0;
		_accu = 0.0f;
		_frames = 0;
		_fps = 0;

		gDefaultMemory = new DefaultAllocator(64 * 1024 * 1024);
		gStringBuffer = new GlobalStringBuffer();
		perf::init();
		repository::initialize(repository::RM_DEBUG);

		JSONReader reader;
		bool ret = reader.parse("content\\engine_settings.json");
		assert(ret);
		int c = reader.find_category("basic_settings");
		assert(c != -1);
		reader.get_int(c, "screen_width", &_settings.screenWidth);
		reader.get_int(c, "screen_height", &_settings.screenHeight);
		reader.get_color(c, "clear_color", &_settings.clearColor);

		LOG << "size: " << _settings.screenWidth << " x " << _settings.screenHeight;
	}


	BaseApp::~BaseApp() {
		repository::shutdown();
		perf::shutdown();
		delete gStringBuffer;
		//gDefaultMemory->printOpenAllocations();
		delete gDefaultMemory;
	}


	bool BaseApp::prepare(HINSTANCE hInstance, HWND hwnd) {
		if (graphics::initialize(hInstance, hwnd, _settings)) {
			JSONReader reader;
			bool ret = reader.parse("content\\resources.json");
			assert(ret);
			int children[256];
			int num = reader.get_categories(children, 256);
			for (int i = 0; i < num; ++i) {
				if (reader.matches(children[i], "quad_index_buffer")) {
					QuadIndexBufferDescriptor descriptor;
					reader.get(children[i], "id", &descriptor.id);
					reader.get(children[i], "size", &descriptor.size);
					graphics::createQuadIndexBuffer(descriptor);
				}
				else if (reader.matches(children[i], "constant_buffer")) {
					ConstantBufferDescriptor descriptor;
					reader.get(children[i], "id", &descriptor.id);
					reader.get(children[i], "size", &descriptor.size);
					graphics::createConstantBuffer(descriptor);
				}
				else if (reader.matches(children[i], "vertex_buffer")) {
					VertexBufferDescriptor descriptor;
					reader.get(children[i], "id", &descriptor.id);
					reader.get(children[i], "size", &descriptor.size);
					reader.get(children[i], "dynamic", &descriptor.dynamic);
					reader.get(children[i], "layout", &descriptor.layout);
					graphics::createVertexBuffer(descriptor);
				}
				else if (reader.matches(children[i], "shader")) {
					ShaderDescriptor descriptor;
					reader.get(children[i], "id", &descriptor.id);
					descriptor.file = reader.get_string(children[i], "file");
					descriptor.vertexShader = reader.get_string(children[i], "vertex_shader");
					descriptor.pixelShader = reader.get_string(children[i], "pixel_shader");
					descriptor.model = reader.get_string(children[i], "shader_model");
					graphics::createShader(descriptor);
				}
				
			}
			return true;
		}
		return false;
	}

	// http://gafferongames.com/game-physics/fix-your-timestep/
	void BaseApp::tick() {
		DWORD now = GetTickCount();
		_delta += (now - _lastTime);
		float elapsed = (float)((now - _lastTime) * 0.001);

		_lastTime = now;
		_accu += elapsed;
		if (_delta > 1000) {
			_delta = 0;
			_fps = _frames;
			_frames = 0;
			//LOG << "FPS:" << _fps;
		}
		while (_accu >= _dt) {
			update(_dt);
			_accu -= _dt;
		}
	}

	void BaseApp::renderFrame() {
		graphics::beginRendering(_settings.clearColor);
		render();
		graphics::endRendering();
		++_frames;
	}

}