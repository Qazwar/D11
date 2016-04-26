#pragma once
#include "..\utils\Color.h"
#include "..\math\math_types.h"
//#include "..\renderer\vertex_types.h"
#include "..\memory\DefaultAllocator.h"

namespace ds {

	// -------------------------------------------------------
	// Particle array
	// -------------------------------------------------------
	struct ParticleArray {

		v3* position;
		//v3* velocity;
		v3* forces;
		v3* normal;
		v2* scale;
		v2* baseScale;
		float* rotation;
		float* rotationVelocity;
		v3* timer;	
		Color* color;
		//SpriteVertex* vertices;
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
			int size = maxParticles * (sizeof(v3) + sizeof(v3) + sizeof(v3) + sizeof(v2) * 2 + sizeof(float) * 2 + sizeof(v3) + sizeof(Color));// +4 * sizeof(PTCVertex));
			buffer = (char*)ALLOC(size);//new char[size];
			position = (v3*)(buffer);
			//velocity = (v3*)(position + maxParticles);
			forces = (v3*)(position + maxParticles);
			normal = (v3*)(forces + maxParticles);
			scale = (Vector2f*)(normal + maxParticles);
			baseScale = (v2*)(scale + maxParticles);
			rotation = (float*)(baseScale + maxParticles);
			rotationVelocity = (float*)(rotation + maxParticles);
			timer = (v3*)(rotationVelocity + maxParticles);
			color = (Color*)(timer + maxParticles);
			//vertices = (PTCVertex*)(color + maxParticles);
			count = maxParticles;
			countAlive = 0;
		}

		void swapData(uint32_t a, uint32_t b) {
			if ( a != b ) {
				position[a] = position[b];
				//velocity[a] = velocity[b];
				forces[a] = forces[b];
				normal[a] = normal[b];
				scale[a] = scale[b];
				baseScale[a] = baseScale[b];
				rotation[a] = rotation[b];
				rotationVelocity[a] = rotationVelocity[b];
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
				swapData(id, countAlive);
				++countAlive;
			}
		}   

	};

}
