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
		_createReport = false;
		_updated = false;
		_running = true;
		
		gDrawCounter = new DrawCounter;
		_buttonState.processed = true;
		_start = std::chrono::steady_clock::now();
		_num = 0;
		_reload = false;
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
		// let the actual app define the settings
		prepare(&_settings);
		// prepare the initial systems
		init_logger(_settings.logTypes, 200, 600);

		createWindow();
		// get system information
		sysinfo::getProcessor(&_systemInfo);
		sysinfo::getGFX(&_systemInfo);
		sysinfo::getRAMInformation(&_systemInfo);
		// FIXME: make sure that we have the amount of memory available
		gDefaultMemory = new DefaultAllocator(_settings.initialMemorySize * 1024 * 1024);
		gStringBuffer = new GlobalStringBuffer();
		perf::init();
		repository::initialize(_settings.repositoryMode);
		_stateMachine = new GameStateMachine;
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

		// now set up the graphic subsystem
		if (graphics::initialize(hInstance, m_hWnd, _settings)) {
			res::initialize(graphics::getDevice());
			graphics::createInternalSpriteBuffer();
			graphics::createPostProcessResources();
			res::parseJSONFile();
			input::init(m_hWnd, _settings.screenWidth, _settings.screenHeight);			
			LOG << "------------------ start load content ------------------";
			loadContent();
			init();
			LOG << "------------------ end load content ------------------";
			res::debug();
			_loading = false;
			_start = std::chrono::steady_clock::now();

			// FIXME: will handle repository reloading - can we do it like this?
			_thread = std::thread(repoReloading,2);
			_thread.detach();
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
			if (_reload) {
				_reload = false;
				repository::reload();
			}
			gDrawCounter->reset();
			perf::reset();
			_updated = false;
			//events::reset();
			tick();
			audio::mix();
			renderFrame();
			perf::finalize();
			// check for internal events
			if (events::num() > 0) {
				for (uint32_t i = 0; i < events::num(); ++i) {
					if (events::getType(i) == InternalEvents::ENGINE_SHUTDOWN) {
						shutdown();
					}
				}
			}
			if (_updated && _createReport) {
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
					//gDefaultMemory->save(rw);
				}
				else {
					LOGE << "Cannot write Report";
				}
				_createReport = false;
			}
		}
	}

	// -------------------------------------------------------
	// send key up
	// -------------------------------------------------------
	void BaseApp::sendKeyUp(WPARAM virtualKey) {
		_keyStates.keyUp = true;
		_keyStates.keyReleased = virtualKey;
		//if (editor::isActive()) {
		gui::sendSpecialKey(virtualKey);
		//}
//#ifdef DEBUG
		if (virtualKey == VK_F1) {
			_createReport = true;
		}		
		else if (virtualKey == VK_F2) {
			_reload = true;
		}
		/*
		else if (virtualKey == VK_F3) {
		m_DebugInfo.showProfiler = !m_DebugInfo.showProfiler;
		m_DebugInfo.profilerTicks = 0;
		m_DebugInfo.snapshotCount = profiler::get_snapshot(_snapshots, 64);
		}
		*/
		else if (virtualKey == VK_F4) {
			_running = !_running;
			LOG << "toggle running: " << _running;
		}
		/*
		else if (virtualKey == VK_F5) {
		m_DebugInfo.performanceOverlay = !m_DebugInfo.performanceOverlay;
		}
		else if (virtualKey == VK_F6) {
		bool ret = editor::toggle();
		m_Running = !ret;
		}
		else if (virtualKey == VK_F7 && !m_DebugInfo.debugRenderer) {
		m_DebugInfo.debugRenderer = true;
		}
		else if (virtualKey == VK_F8) {
		m_DebugInfo.showActionBar = !m_DebugInfo.showActionBar;
		}
		else if (virtualKey == VK_F9) {
		m_DebugInfo.showConsole = !m_DebugInfo.showConsole;
		}
		*/
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
	void BaseApp::tick() {
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
		_now = std::chrono::steady_clock::now();
		auto duration = _now - _start;
		auto time_span = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
		double elapsed = static_cast<double>(time_span) / 1000.0f / 1000.0f;
		_ar[_num++] = elapsed;
		if (_num >= 255) {
			_num = 0;
		}
		_start = _now;
		_accu += elapsed;
		perf::tickFPS(elapsed);
		{
			ZoneTracker u1("UPDATE");
			while (_accu >= _dt) {
				if (_running) {
					{
						ZoneTracker u2("UPDATE::main");
						update(_accu);
					}
					_stateMachine->update(_accu);
					// updating particles
					ParticleManager* pm = res::getParticleManager();
					if (pm != 0) {
						pm->update(_accu);
					}
				}
				_accu -= _dt;
				_updated = true;
			}		
		}		
		if (_updated) {
			events::reset();
		}
	}

	// -------------------------------------------------------
	// render frame
	// -------------------------------------------------------
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