#pragma once
#include "..\gamestates\GameState.h"
#include "..\renderer\render_types.h"
#include "ParticleManager.h"

namespace ds {

	struct ParticlesTestSettings {

		RID blendState;
		bool render;
		v2 screenSize;

		ParticlesTestSettings() : blendState(0), render(true), screenSize(1024, 768) {}
	};

	class ParticlesTestState : public GameState {

	public:
		ParticlesTestState(const ParticlesTestSettings& settings);
		virtual ~ParticlesTestState();
		void init();
		int update(float dt);
		void render();
		void activate();
		void deactivate();
		int onChar(int ascii);
	private:
		ParticlesTestSettings _settings;
		ParticleManager* _particles;
		int _index;
		int _num;
		int _ids[64];
		int _states[64];

	};

}