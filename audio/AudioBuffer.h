#pragma once
#include "Sound.h"
#include <dsound.h>

namespace ds {

class AudioBuffer {

public:
	AudioBuffer(LPDIRECTSOUNDBUFFER sample, Sound *sound);
	~AudioBuffer();	
	bool play(int volume, bool looping);	
	bool pause();
	bool stop();
	bool resume();
	bool isPlaying();
	bool isLooping() { return m_Looping;};
	void setVolume(int volume);
	int getVolume() { return m_Volume;}
private:
	bool FillBufferWithSound();
	LPDIRECTSOUNDBUFFER m_Sample;
	Sound *sound;
	int m_Volume;
	bool m_Looping;	
};

};

