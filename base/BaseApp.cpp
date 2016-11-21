#include "BaseApp.h"
#include "..\renderer\graphics.h"
#include "core\log\Log.h"
#include "core\io\json.h"
#include "core\string\GlobalStringBuffer.h"
#include "core\io\FileRepository.h"
#include "core\profiler\Profiler.h"
#include "..\imgui\IMGUI.h"
#include "InputSystem.h"
#include <strsafe.h>
#include "core\data\DynamicSettings.h"
#include "..\stats\DrawCounter.h"
#include <thread>
#include "..\audio\AudioManager.h"

void ErrorExit(LPTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(dw);
}

namespace ds {

	// ----------------------------------------------------
	// Thread function to call repository::reload
	// ----------------------------------------------------
	void repoReloading(int waitSeconds) {
		long long wait = waitSeconds * 1000;
		while (1) {
			std::this_thread::sleep_for(std::chrono::milliseconds(wait));
			repository::reload();
		}
	}

	BaseApp::BaseApp() {
		_alive = true;
		_dt = 1.0f / 60.0f;
		_accu = 0.0f;
		_loading = true;
		_debugInfo.createReport = false;
		_debugInfo.updated = false;
		_debugInfo.showPerfHud = false;
		_debugInfo.showGameStateDialog = false;
		_running = true;
		
		gDrawCounter = new DrawCounter;
		_buttonState.processed = true;
		_start = std::chrono::steady_clock::now();

		gDefaultMemory = new DefaultAllocator(_settings.initialMemorySize * 1024 * 1024);
	}


	BaseApp::~BaseApp() {
		audio::shutdown();
		repository::shutdown();
		perf::shutdown();		
		events::shutdown();
		input::shutdown();
		res::shutdown();
		delete _shortcuts;
		delete gDrawCounter;
		delete _stateMachine;
		delete gStringBuffer;		
		graphics::shutdown();
		//gDefaultMemory->printOpenAllocations();		
		delete gDefaultMemory;		
		shutdown_logger();
	}

	// -------------------------------------------------------
	// Creates the window
	// -------------------------------------------------------
	void BaseApp::createWindow() {
		RECT DesktopSize;
		GetClientRect(GetDesktopWindow(), &DesktopSize);
		// Create the application's window
		m_hWnd = CreateWindow("D11", "D11",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			(DesktopSize.right - _settings.screenWidth) / 2, (DesktopSize.bottom - _settings.screenHeight) / 2,
			_settings.screenWidth, _settings.screenHeight,
			NULL, NULL, hInstance, NULL);

		if (!m_hWnd) 	{
			DWORD dw = GetLastError();
			LOG << "Failed to created window";
			ErrorExit(TEXT("CreateWindow"));
			ExitProcess(dw);
		}

		// Adjust to desired size
		RECT rect = { 0, 0, _settings.screenWidth, _settings.screenHeight };
		AdjustWindowRect(&rect, GetWindowLong(m_hWnd, GWL_STYLE), FALSE);
		SetWindowPos(m_hWnd, HWND_TOP, 0, 0, rect.right - rect.left, rect.bottom - rect.top,
			SWP_NOZORDER | SWP_NOMOVE);

		LOG << "window rect " << rect.top << " " << rect.left << " " << rect.bottom << " " << rect.right;
		ShowWindow(m_hWnd, SW_SHOW);
		UpdateWindow(m_hWnd);

		SetWindowText(m_hWnd, getTitle());

		// Save current location/size
		//ZeroMemory(&m_wp, sizeof(WINDOWPLACEMENT));
		//m_wp.length = sizeof(WINDOWPLACEMENT);
		//GetWindowPlacement(m_hWnd, &m_wp);
		LOG << "window created";
	}


	bool BaseApp::prepare() {
		//gDefaultMemory = new DefaultAllocator(_settings.initialMemorySize * 1024 * 1024);
		// let the actual app define the settings
		prepare(&_settings);
		// prepare the initial systems
		init_logger();

		createWindow();
		// get system information
		sysinfo::getProcessor(&_systemInfo);
		sysinfo::getGFX(&_systemInfo);
		sysinfo::getRAMInformation(&_systemInfo);
		// FIXME: make sure that we have the amount of memory available
		
		gStringBuffer = new GlobalStringBuffer();
		perf::init();
		repository::initialize(_settings.repositoryMode);		
		_shortcuts = new ShortcutsHandler();
		events::init();
		math::init_random(GetTickCount());
		audio::initialize(m_hWnd);
		LOG << "---------- System information ----------";
		LOG << "Processor : " << _systemInfo.processor;
		LOG << "Speed     : " << _systemInfo.mhz;
		LOG << "GPU Model : " << _systemInfo.gpuModel;
		LOG << "Total RAM : " << _systemInfo.total_memory_MB;
		LOG << "Free  RAM : " << _systemInfo.free_memory_MB;
		LOG << "---------- Keys ------------------------";
		LOG << "F1 = Save report";
		LOG << "F2 = toggle perf HUD";
		LOG << "F3 = toggle game state dialog";
		LOG << "F4 = toggle update";
		// now set up the graphic subsystem
		if (graphics::initialize(hInstance, m_hWnd, _settings)) {
			res::initialize(graphics::getDevice());
			graphics::createBlendStates();
			graphics::createInternalSpriteBuffer();
			graphics::createPostProcessResources();
			res::parseJSONFile();
			input::init(m_hWnd, _settings.screenWidth, _settings.screenHeight);			
			_stateMachine = new GameStateMachine;
			LOG << "------------------ start load content ------------------";
			loadContent();
			init();
			LOG << "------------------ end load content ------------------";
			res::debug();
			_loading = false;
			_start = std::chrono::steady_clock::now();

			// FIXME: will handle repository reloading - can we do it like this?
			if (_settings.reloading) {
				_thread = std::thread(repoReloading, 2);
				_thread.detach();
			}
			_shortcuts->debug();
			events::reset();
			return true;
		}			
		_shortcuts->debug();
		events::reset();
		_loading = false;
		return false;
	}

	void BaseApp::buildFrame() {
		if (_alive) {
			gDrawCounter->reset();
			perf::reset();
			_debugInfo.updated = false;
			_now = std::chrono::steady_clock::now();
			auto duration = _now - _start;
			auto time_span = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
			double elapsed = static_cast<double>(time_span) / 1000.0f / 1000.0f;
			_start = _now;
			//events::reset();
			tick(elapsed);
			{
				ZoneTracker az("Audio:mix");
				audio::mix();
			}
			renderFrame();
			perf::finalize();
			perf::addTimerValue("Duration", perf::get_current_total_time());
			if (perf::get_current_total_time() > 10.0f && _debugInfo.updated) {
				_debugInfo.createReport = true;
			}
			// check for internal events
			if (events::num() > 0) {
				for (uint32_t i = 0; i < events::num(); ++i) {
					if (events::getType(i) == InternalEvents::ENGINE_SHUTDOWN) {
						shutdown();
					}
				}
			}
			if (_debugInfo.updated && _debugInfo.createReport) {
				saveReport();
				_debugInfo.createReport = false;
			}
		}
	}

	void BaseApp::saveReport() {
		
		char timeFormat[255];
		time_t now;
		time(&now);
		tm *now_tm = localtime(&now);
		strftime(timeFormat, 255, "%Y%m%d_%H%M%S", now_tm);
		char filename[255];
		sprintf_s(filename, "%s\\%s.html", _settings.reportingDirectory, timeFormat);
		ReportWriter rw(filename);
		if (rw.isOpen()) {
			gDrawCounter->save(rw);
			perf::save(rw);
			res::save(rw);
			gDefaultMemory->save(rw);
		}
		else {
			LOGE << "Cannot write Report";
		}
	}
	// -------------------------------------------------------
	// send key up
	// -------------------------------------------------------
	void BaseApp::sendKeyUp(WPARAM virtualKey) {
		_keyStates.keyUp = true;
		_keyStates.keyReleased = virtualKey;
		gui::sendSpecialKey(virtualKey);
//#ifdef DEBUG
		if (virtualKey == VK_F1) {
			_debugInfo.createReport = true;
		}		
		else if (virtualKey == VK_F2) {
			_debugInfo.showPerfHud = !_debugInfo.showPerfHud;
		}
		else if (virtualKey == VK_F3) {
			_debugInfo.showGameStateDialog = !_debugInfo.showGameStateDialog;
		}
		else if (virtualKey == VK_F4) {
			_running = !_running;
			LOG << "toggle running: " << _running;
		}
//#endif
	}

	// -------------------------------------------------------
	// send key down
	// -------------------------------------------------------
	void BaseApp::sendKeyDown(WPARAM virtualKey) {
		_keyStates.keyDown = true;
		_keyStates.keyPressed = virtualKey;

	}

	void BaseApp::addShortcut(const char* label, char key, uint32_t eventType) {
		_shortcuts->add(label, key, eventType);
	}

	// -------------------------------------------------------
	// tick
	// -------------------------------------------------------
	// http://gafferongames.com/game-physics/fix-your-timestep/
	void BaseApp::tick(double elapsed) {
		ZoneTracker all("tick");
		{
			ZoneTracker z("INPUT");
			if (_running) {
				if (_keyStates.onChar) {
					_keyStates.onChar = false;
					_stateMachine->onChar(_keyStates.ascii);
					_shortcuts->handleInput(_keyStates.ascii);
					OnChar(_keyStates.ascii);
				}
				if (!_buttonState.processed) {
					_buttonState.processed = true;
					if (_buttonState.down) {
						_stateMachine->onButtonDown(_buttonState.button, _buttonState.x, _buttonState.y);
						OnButtonDown(_buttonState.button, _buttonState.x, _buttonState.y);
					}
					else {
						_stateMachine->onButtonUp(_buttonState.button, _buttonState.x, _buttonState.y);
						OnButtonUp(_buttonState.button, _buttonState.x, _buttonState.y);
					}
				}
			}
		}
		
		_accu += elapsed;
		perf::tickFPS(elapsed);
		{
			ZoneTracker u1("UPDATE");
			int uc = 0;
			while (_accu >= _dt) {
				if (_running) {
					{
						ZoneTracker u2("UPDATE::main");
						update(_dt);
					}
					_stateMachine->update(_dt);
					// updating particles
					ParticleManager* pm = res::getParticleManager();
					if (pm != 0) {
						pm->update(_dt);
					}
				}
				_accu -= _dt;
				_debugInfo.updated = true;
				++uc;
			}		
			if (uc > 2) {
				LOG << "uc: " << uc;
			}
		}		
		if (_debugInfo.updated) {
			events::reset();
		}
	}

	// -------------------------------------------------------
	// render frame
	// -------------------------------------------------------
	float niceNum(float range, bool round) {
		float exponent = floor(std::log10(range));
		float fraction = range / std::pow(10, exponent);
		float niceFraction = 0.0f;
		bool niceRound = true;
		if (niceRound) {
			if (fraction < 1.5f)
				niceFraction = 1.0f;
			else if (fraction < 3.0f)
				niceFraction = 2.0f;
			else if (fraction < 7.0f)
				niceFraction = 5.0f;
			else
				niceFraction = 10.0f;
		}
		else {
			if (fraction <= 1.0f)
				niceFraction = 1.0f;
			else if (fraction <= 2.0f)
				niceFraction = 2.0f;
			else if (fraction <= 5.0f)
				niceFraction = 5.0f;
			else
				niceFraction = 10.0f;
		}
		return niceFraction * std::pow(10, exponent);
	}

	void BaseApp::renderFrame() {
		ZoneTracker("Render");
		graphics::beginRendering();
		{
			ZoneTracker("Render::render");
			render();
		}				
		{
			ZoneTracker("Render::stateMachine");
			_stateMachine->render();
		}
		if (_debugInfo.showPerfHud) {			
			v2 p(10, graphics::getScreenHeight() - 10.0f);
			gui::start(1, &p);
			int state = 1;
			gui::begin("Perf HUD",&state);
			float values[16];
			int num = perf::getTimerValues("Duration",values, 16);
			float min = 100000.0f;
			float max = 0.0f;
			for (int i = 0; i < num; ++i) {
				if (values[i] > max) {
					max = values[i];
				}
				if (values[i] < min) {
					min = values[i];
				}
			}
			float tickSpacing = 0.1f;
			float niceMin = std::floor(min);
			float niceMax = std::ceil(max);
			gui::Histogram(values, num, niceMin, niceMax, tickSpacing);
			gui::end();
		}
		if (_debugInfo.showGameStateDialog) {
			_stateMachine->showDialog();
		}
		{
			ZoneTracker("Render::endFrame");
			gui::endFrame();
			graphics::endRendering();
			perf::incFrame();
		}
	}

	// -------------------------------------------------------
	// Key message handling
	// -------------------------------------------------------
	void BaseApp::sendOnChar(char ascii, unsigned int state) {
		_keyStates.ascii = ascii;
		_keyStates.onChar = true;
		gui::sendKey(ascii);
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

	// -------------------------------------------------------
	// Mouse button
	// -------------------------------------------------------
	void BaseApp::sendButton(int button, int x, int y, bool down) {
		if (_buttonState.processed) {			
			_buttonState.processed = false;
			_buttonState.x = x;
			_buttonState.y = graphics::getScreenHeight() - y;
			_buttonState.button = button;
			_buttonState.down = down;
		}
	}

	void BaseApp::shutdown() {
		_alive = false;
		onShutdown();
		//DestroyWindow(m_hWnd);
	}

	bool BaseApp::isRunning() const {
		return _alive;
	}

}