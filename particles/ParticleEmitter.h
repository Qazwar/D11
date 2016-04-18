#pragma once
#include "Particle.h"
#include "..\Common.h"
#include "..\lib\DataArray.h"

namespace ds {

	const int MAX_SPAWNERS = 1024;
	// -------------------------------------------------------
	// Particle spawner
	// -------------------------------------------------------
	struct ParticleSpawner {
		
		float duration;
		int rate;
		int loop;
		float loopDelay;
		float frequency;
		bool sendEvents;
	};

	// -------------------------------------------------------
	// Particle spawner instance
	// -------------------------------------------------------
	struct ParticleSpawnerInstance {
		ID id;
		float timer;
		float accumulated;
		v2 pos;
		int loop;
		float loopDelay;
		float loopTimer;
		float ttl;

		ParticleSpawnerInstance() : id(INVALID_ID), timer(0.0f), accumulated(0.0f), pos(0, 0), loop(0), ttl(0.0f) , loopTimer(0.0f) {}
	};

	typedef DataArray<ParticleSpawnerInstance, MAX_SPAWNERS> SpawnerInstances;

}


