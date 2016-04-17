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
		_accu = 0.0f;
		_frames = 0;
		_fps = 0;
		_fpsTimer = 0.0f;
		_loading = true;
		_createReport = false;
		_updated = false;
		gDefaultMemory = new DefaultAllocator(64 * 1024 * 1024);
		gStringBuffer = new GlobalStringBuffer();
		perf::init();
		repository::initialize(repository::RM_DEBUG);
		_stateMachine = new GameStateMachine;
		JSONReader reader;
		bool ret = reader.parse("content\\engine_settings.json");
		assert(ret);
		int c = reader.find_category("basic_settings");
		assert(c != -1);
		reader.get_int(c, "screen_width", &_settings.screenWidth);
		reader.get_int(c, "screen_height", &_settings.screenHeight);
		reader.get_color(c, "clear_color", &_settings.clearColor);
		if (reader.contains_property(c, "reporting_directory")) {
			const char* dir = reader.get_string(c, "reporting_directory");
			sprintf_s(_settings.reportingDirectory, 64, "%s\\", dir);
		}
		else {
			sprintf_s(_settings.reportingDirectory, "");
		}
		LOG << "size: " << _settings.screenWidth << " x " << _settings.screenHeight;
		_start = std::chrono::steady_clock::now();
	}


	BaseApp::~BaseApp() {
		repository::shutdown();
		perf::shutdown();
		delete _stateMachine;
		delete gStringBuffer;
		res::shutdown();
		//gDefaultMemory->printOpenAllocations();
		delete gDefaultMemory;
	}


	bool BaseApp::prepare(HINSTANCE hInstance, HWND hwnd) {
		if (graphics::initialize(hInstance, hwnd, _settings)) {
			profiler::init();
			res::initialize(graphics::getDevice());
			res::parseJSONFile();
			_loading = false;
			return true;
		}
		_loading = false;
		return false;
	}

	void BaseApp::buildFrame() {
		profiler::reset();
		perf::reset();
		tick();
		renderFrame();
		profiler::finalize();
		perf::finalize();
		if (_updated && _createReport) {
			char timeFormat[255];
			time_t now;
			time(&now);
			tm *now_tm = localtime(&now);
			strftime(timeFormat, 255, "%Y%m%d_%H%M%S", now_tm);
			char filename[255];
			sprintf_s(filename, "%s%s.html", _settings.reportingDirectory, timeFormat);
			ReportWriter rw(filename);
			perf::save(rw);
			gDefaultMemory->save(rw);
			_createReport = false;
		}
	}

	// -------------------------------------------------------
	// tick
	// -------------------------------------------------------
	// http://gafferongames.com/game-physics/fix-your-timestep/
	void BaseApp::tick() {
		{
			ZoneTracker z("INPUT");
			if (_keyStates.onChar) {
				_keyStates.onChar = false;
				OnChar(_keyStates.ascii);				
			}
		}
		_now = std::chrono::steady_clock::now();
		auto duration = _now - _start;
		auto time_span = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
		double elapsed = static_cast<double>(time_span) / 1000.0;
		_start = _now;
		_accu += elapsed;
		_fpsTimer += elapsed;
		if (_fpsTimer >= 1.0f) {
			_fpsTimer -= 1.0f;
			_fps = _frames;
			_frames = 0;
			//LOG << "FPS:" << _fps;
		}
		ZoneTracker u1("UPDATE");
		{
			while (_accu >= _dt) {
				update(_dt);
				_stateMachine->update(_dt);
				_accu -= _dt;
				_updated = true;
			}
		}
		
	}

	// -------------------------------------------------------
	// render frame
	// -------------------------------------------------------
	void BaseApp::renderFrame() {
		ZoneTracker("Render");
		graphics::beginRendering(_settings.clearColor);
		_stateMachine->render();
		render();
		graphics::endRendering();
		++_frames;
	}

	// -------------------------------------------------------
	// Key message handling
	// -------------------------------------------------------
	void BaseApp::sendOnChar(char ascii, unsigned int state) {
		_keyStates.ascii = ascii;
		_keyStates.onChar = true;
		if (ascii == 'r') {
			_createReport = true;
		}
	}

	// -------------------------------------------------------
	// add game state
	// -------------------------------------------------------
	void BaseApp::addGameState(GameState* gameState) {
		_stateMachine->add(gameState);
	}

	// -------------------------------------------------------
	// activate game state
	// -------------------------------------------------------
	void BaseApp::activate(const char* name) {
		_stateMachine->activate(name);
	}

	// -------------------------------------------------------
	// deactivate game state
	// -------------------------------------------------------
	void BaseApp::deactivate(const char* name) {
		_stateMachine->deactivate(name);
	}


	// -------------------------------------------------------
	// connect game states
	// -------------------------------------------------------
	void BaseApp::connectGameStates(const char* firstStateName, int outcome, const char* secondStateName) {
		_stateMachine->connect(firstStateName, outcome, secondStateName);
	}

}