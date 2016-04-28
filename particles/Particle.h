#pragma once
#include "..\utils\Color.h"
#include "..\math\math_types.h"
#include "..\memory\DefaultAllocator.h"
#include "..\utils\Log.h"

namespace ds {

	// -------------------------------------------------------
	// Particle array
	// -------------------------------------------------------
	struct ParticleArray {

		uint32_t* ids;
		v3* position;
		v3* forces;
		v3* normal;
		v2* scale;
		float* rotation;
		v3* timer;	
		Color* color;
		char* buffer;

		uint32_t count;
		uint32_t countAlive;

		ParticleArray() : count(0) , countAlive(0) , buffer(0) {}

		~ParticleArray() {		
			if ( buffer != 0 ) {
				//delete[] buffer;
				DEALLOC(buffer);
			}
		}

		void initialize(unsigned int maxParticles) {
			int size = maxParticles * (sizeof(uint32_t) + sizeof(v3) + sizeof(v3) + sizeof(v3) + sizeof(v2) + sizeof(float) + sizeof(v3) + sizeof(Color));
			buffer = (char*)ALLOC(size);
			ids = (uint32_t*)(buffer);
			position = (v3*)(ids + maxParticles);
			forces = (v3*)(position + maxParticles);
			normal = (v3*)(forces + maxParticles);
			scale = (Vector2f*)(normal + maxParticles);
			rotation = (float*)(scale + maxParticles);
			timer = (v3*)(rotation + maxParticles);
			color = (Color*)(timer + maxParticles);
			count = maxParticles;
			countAlive = 0;
		}

		void swapData(uint32_t a, uint32_t b) {
			//LOG << "swapping from " << b << " (" << ids[b] << ") to " << a << " (" << ids[a] << ")";
			if ( a != b ) {				
				ids[a] = ids[b];
				position[a] = position[b];
				forces[a] = forces[b];
				normal[a] = normal[b];
				scale[a] = scale[b];
				rotation[a] = rotation[b];
				timer[a] = timer[b];
				color[a] = color[b];
			}
		}

		void kill(uint32_t id) {
			if (countAlive > 0) {
				swapData(id, countAlive - 1);
				--countAlive;
			}
		}

		void wake(uint32_t id) {
			if (countAlive < count)	{
				//swapData(id, countAlive);
				++countAlive;
			}
		}   

	};

}
