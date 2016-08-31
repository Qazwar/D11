#include "AudioBuffer.h"
#include "core\log\Log.h"

//#define GCC_DSBVOLUME_MIN (DSBVOLUME_MIN)
#define GCC_DSBVOLUME_MIN (-2000)

namespace ds {

AudioBuffer::AudioBuffer(LPDIRECTSOUNDBUFFER sample, Sound *sound) {
	m_Sample = sample;
	this->sound = sound;
	bool ret = FillBufferWithSound();
	if ( !ret ) {
		LOGE <<"Failed to fill buffer with sound";
	}
}


AudioBuffer::~AudioBuffer(void) {
	//SAFE_RELEASE(m_Sample);
}

bool AudioBuffer::play(int volume, bool looping) {
	stop();
	m_Volume = volume;
	m_Looping = looping;
	if (!m_Sample)
		return false;

    //m_Sample->SetVolume( volume );
	setVolume( volume );
    
    DWORD dwFlags = looping ? DSBPLAY_LOOPING : 0L;
	HRESULT hr = m_Sample->Play( 0, 0, dwFlags );
	if ( hr == S_OK ) {
		return true;
	}
	return false;
}

bool AudioBuffer::FillBufferWithSound() {
	VOID	*pDSLockedBuffer = NULL;	 // a pointer to the DirectSound buffer
    DWORD   dwDSLockedBufferSize = 0;    // Size of the locked DirectSound buffer
    DWORD   dwWavDataRead        = 0;    // Amount of data read from the wav file 

	if( m_Sample == NULL ) {
		LOGE << "No sample!";
        return false;
	}

    /*
    if( FAILED( hr = RestoreBuffer( NULL ) ) ) 
        return DXUT_ERR( L"RestoreBuffer", hr );
		*/
	int pcmBufferSize = sound->GetPCMBufferSize();
    // Lock the buffer down
    HRESULT hr = m_Sample->Lock( 0, pcmBufferSize, 
                                 &pDSLockedBuffer, &dwDSLockedBufferSize, 
                                 NULL, NULL, 0L );
	if ( hr != S_OK ) {
		LOGE <<"Failed to lock sample buffer";
		return false;
	}     
	/*
    if( pcmBufferSize == 0 ) {
        // Wav is blank, so just fill with silence
        FillMemory( (BYTE*) pDSLockedBuffer, 
                    dwDSLockedBufferSize, 
                    (BYTE)(sound->GetFormat()->wBitsPerSample == 8 ? 128 : 0 ) );
    }
    else  {
		CopyMemory(pDSLockedBuffer, sound->GetPCMBuffer(), pcmBufferSize);
		if( pcmBufferSize < (int)dwDSLockedBufferSize ) {
            // If the buffer sizes are different fill in the rest with silence 
            FillMemory( (BYTE*) pDSLockedBuffer + pcmBufferSize, 
                        dwDSLockedBufferSize - pcmBufferSize, 
                        (BYTE)(sound->GetFormat()->wBitsPerSample == 8 ? 128 : 0 ) );
        }
    }
	*/
    m_Sample->Unlock( pDSLockedBuffer, dwDSLockedBufferSize, NULL, 0 );
    return true;
}

bool AudioBuffer::pause() {
	return true;
}

bool AudioBuffer::stop() {
	m_Sample->Stop();
	m_Sample->SetCurrentPosition(0);
	return true;
}

bool AudioBuffer::resume() {
	return true;
}

bool AudioBuffer::isPlaying() {
	DWORD dwStatus = 0;
	m_Sample->GetStatus( &dwStatus );
    bool bIsPlaying = ( ( dwStatus & DSBSTATUS_PLAYING ) != 0 );
	return bIsPlaying;
}

void AudioBuffer::setVolume(int volume) {
	// GCC_DSBVOLUME_MIN is defined as -5000 - why? Because DSBVOLUME_MIN, defined in dsound.h 
	// is set to as -10000, which is just way too silent for a lower bound and many programmers find 
	// -5000 to be a better minimum bound for the volume range to avoid an annoying silence for the 
	// lower 50% of a volume slider that uses a logarithmic scale.
	m_Volume = volume;
	//assert(volume>=0 && volume<=100 && "Volume must be a number between 0 and 100");
	// convert volume from 0-100 into range for DirectX - don't forget to use a logarithmic scale!
	float coeff = (float)m_Volume / 100.0f;
	float logarithmicProportion = coeff >0.1f  ? 1+log10(coeff)  : 0;
	float range = (DSBVOLUME_MAX - GCC_DSBVOLUME_MIN);
	float fvolume = ( range * logarithmicProportion ) + GCC_DSBVOLUME_MIN;
	//assert(fvolume>=GCC_DSBVOLUME_MIN && fvolume<=DSBVOLUME_MAX);
	m_Sample->SetVolume(LONG(fvolume));
	//assert(hr==S_OK);
}

};
