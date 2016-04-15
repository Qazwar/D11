#pragma once
#include <Windows.h>
#include "Settings.h"
#include "..\resources\ResourceContainer.h"
#include "..\gamestates\GameStateMachine.h"

namespace ds {

	class BaseApp {

		struct KeyStates {
			uint8_t ascii;
			bool onChar;
			WPARAM keyPressed;
			bool keyDown;
			WPARAM keyReleased;
			bool keyUp;
		};

	public:
		BaseApp();
		~BaseApp();
		bool prepare(HINSTANCE hInstance, HWND hwnd);
		virtual bool initialize() = 0;
		virtual void render() = 0;
		virtual void update(float dt) = 0;		
		void buildFrame();
		bool isLoading() const {
			return _loading;
		}
		void sendOnChar(char ascii, unsigned int state);
	protected:
		void addGameState(GameState* gameState);
		void activate(const char* name);
		void deactivate(const char* name);
		void connectGameStates(const char* firstStateName, int outcome, const char* secondStateName);
		virtual void OnChar(uint8_t ascii) {}
	private:
		void tick();
		void renderFrame();
		float _dt;
		DWORD _lastTime;
		DWORD _delta;
		float _accu;
		int _frames;
		int _fps;
		bool _loading;
		Settings _settings;
		ResourceContainer* _resources;
		GameStateMachine* _stateMachine;
		KeyStates _keyStates;
		bool _createReport;
		bool _updated;
	};

}