#pragma once
#include "core\Common.h"
#include "core\string\StaticHash.h"
//#include <io.h>
#include <stdio.h>

namespace ds {

class Sound {

public:
	Sound(const StaticHash& name);
	virtual ~Sound();
	bool load(const char* fileName);	
	int loadWavFile(const char* filename);
	char const *GetPCMBuffer() const { return m_PCMBuffer; }
	int GetPCMBufferSize() const { return m_PCMBufferSize; }	
	//WAVEFORMATEX const *GetFormat() { return &m_WavFormatEx; }
	int GetLengthMilli() const { return m_LengthMilli; }
	const StaticHash& getName() const {
		return m_Name;
	}
private:
	StaticHash m_Name;
	bool parseWave(FILE* file);
	bool m_bInitialized;
	char *m_PCMBuffer;
	int m_PCMBufferSize;
	//WAVEFORMATEX m_WavFormatEx;
	int m_LengthMilli;
	//WORD channels;
  	//WORD bits;
  	//WORD sampleRate;
};

};