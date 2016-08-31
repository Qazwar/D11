#pragma once
#include <dsound.h>
#include <mmsystem.h>
#include "core\Common.h"
#include "AudioBuffer.h"
#include <map>
#include "core\lib\collection_types.h"
#include "core\string\StaticHash.h"
#include "Sound.h"

namespace ds {

	namespace audio {

		bool initialize(HWND hwnd);

		void shutdown();

		void load(const StaticHash& hash);

	}

	class AudioBuffer;
	// -------------------------------------------------------
	// AudioManager
	//
	// Loads and manages sounds. Also plays sounds.
	// -------------------------------------------------------
	class AudioManager {

	struct AudioSound {
		StaticHash hash;
		char name[32];
		AudioBuffer* buffer;
	};

	typedef Array<AudioSound> AudioBuffers;

	public:
		AudioManager();
		~AudioManager(void);
		bool initialize(HWND hWnd);	
		bool play(const char* soundName,int volume = 100, bool looping = false);
		void stop(const char* soundName);
		bool isPlaying(const char* soundName);
		//
		void loadSound(const char* name);
	private:
		bool createAudioBuffer(Sound* sound);
		AudioBuffer* findByName(const char* name);
		bool setPrimaryBufferFormat(DWORD dwPrimaryChannels,DWORD dwPrimaryFreq,DWORD dwPrimaryBitRate );
		bool m_Initialized;
		IDirectSound8* m_pDS;
		AudioBuffers m_AudioBuffers;

	};

};

