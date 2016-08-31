#include "AudioManager.h"
#include <comdef.h>
#include "core\log\Log.h"
#include "core\string\StringUtils.h"
#include "core\io\FileRepository.h"
#include <stdint.h>

namespace ds {

	namespace audio {

		struct AudioContext {

			IDirectSound8* directSound;
			Array<AudioBuffer> audioBuffers;

		};

		static AudioContext* audioCtx = nullptr;

		bool initialize(HWND hwnd) {
			audioCtx = new AudioContext;
			HRESULT hr = DirectSoundCreate8(NULL, &audioCtx->directSound, NULL);

			hr = audioCtx->directSound->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);

			LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;

			// Get the primary buffer 
			DSBUFFERDESC dsbd;
			ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
			dsbd.dwSize = sizeof(DSBUFFERDESC);
			dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER;
			dsbd.dwBufferBytes = 0;
			dsbd.lpwfxFormat = NULL;

			hr = audioCtx->directSound->CreateSoundBuffer(&dsbd, &pDSBPrimary, NULL);

			WAVEFORMATEX wfx;
			ZeroMemory(&wfx, sizeof(WAVEFORMATEX));
			wfx.wFormatTag = (WORD)WAVE_FORMAT_PCM;
			wfx.nChannels = (WORD)2;
			wfx.nSamplesPerSec = (DWORD)22050;
			wfx.wBitsPerSample = (WORD)16;
			wfx.nBlockAlign = (WORD)(wfx.wBitsPerSample / 8 * wfx.nChannels);
			wfx.nAvgBytesPerSec = (DWORD)(wfx.nSamplesPerSec * wfx.nBlockAlign);

			if (FAILED(hr = pDSBPrimary->SetFormat(&wfx))) {
				LOGE << "Error while setting format";
				return false;
			}
			if (pDSBPrimary != 0) {
				pDSBPrimary->Release();
			}
			return true;

		}

		void shutdown() {
			if (audioCtx != nullptr) {
				audioCtx->directSound->Release();
				delete audioCtx;
			}
		}

		struct  WaveHeader {
			char                RIFF[4];        // RIFF Header      Magic header
			unsigned long       ChunkSize;      // RIFF Chunk Size
			char                WAVE[4];        // WAVE Header
			char                fmt[4];         // FMT header
			unsigned long       Subchunk1Size;  // Size of the fmt chunk
			unsigned short      AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
			unsigned short      NumOfChan;      // Number of channels 1=Mono 2=Sterio
			unsigned long       SamplesPerSec;  // Sampling Frequency in Hz
			unsigned long       bytesPerSec;    // bytes per second
			unsigned short      blockAlign;     // 2=16-bit mono, 4=16-bit stereo
			unsigned short      bitsPerSample;  // Number of bits per sample
			char                Subchunk2ID[4]; // "data"  string
			unsigned long       Subchunk2Size;  // Sampled data length
		};
		// http://rogerchansdigitalworld.blogspot.de/2010/05/how-to-read-wav-format-file-in-c.html
		// http://stackoverflow.com/questions/13660777/c-reading-the-data-part-of-a-wav-file
		// http://stackoverflow.com/questions/16075233/reading-and-processing-wav-file-data-in-c-c
		void load(const StaticHash& hash) {
			Sound *s = new Sound(hash);
			int size = -1;
			//LOG << "loading sound " << fileName;
			char* data = repository::load(hash, &size, repository::FileType::FT_BINARY);
			WaveHeader header;
			memcpy(&header, data, sizeof(WaveHeader));
			LOG << "Magic header: " << header.RIFF;
			LOG << "Wave header: " << header.WAVE;
			LOG << "FMT header: " << header.fmt;
			LOG << "FMT chunk: " << header.Subchunk1Size;
			LOG << "format: " << header.AudioFormat;
			LOG << "Samples: " << header.SamplesPerSec;
			LOG << "Bits: " << header.bitsPerSample;
			LOG << "Subchunk2: " << header.Subchunk2ID;
			LOG << "Sub2 size: " << header.Subchunk2Size;
			//int ret = s->loadWavFile(fileName);
			//if (ret > 0) {
				//createAudioBuffer(s);
			//}
			//else {
				//LOGE << "Unable to load wav file";
			//}
			//delete s;
			delete[] data;
		}

	}



AudioManager::AudioManager(void) {
	m_Initialized = false;
}

// -------------------------------------------------------
// Destructor - removes all audio buffers
// -------------------------------------------------------
AudioManager::~AudioManager(void) {
	for ( uint32_t i = 0; i < m_AudioBuffers.size();++i ) {
		LOG << "removing sound " << m_AudioBuffers[i].name;
		delete m_AudioBuffers[i].buffer;
	}
	if (m_pDS != 0) {
		delete m_pDS;
	}
}

// -------------------------------------------------------
// Initializes AudioManager
// -------------------------------------------------------
bool AudioManager::initialize(HWND hWnd) {	
	LOG << "Initialize";
	if(m_Initialized) {
		LOGE << "Already initialized";
		return true;
	}
	m_Initialized=false;

	//SAFE_RELEASE( m_pDS );

	HRESULT hr;

	// Create IDirectSound using the primary sound device
	if( FAILED( hr = DirectSoundCreate8( NULL, &m_pDS, NULL ) ) ) {
		LOGE << "Failed to create direct sound";
		return false;
	}

	// Set DirectSound coop level 
	if( FAILED( hr = m_pDS->SetCooperativeLevel( hWnd, DSSCL_PRIORITY) ) ) {
		LOGE << "Failed to set cooperative level";
		return false;
	}
	//if( !setPrimaryBufferFormat( 8, 44100, 16 ) )
	if( !setPrimaryBufferFormat( 2, 22050, 16 ) ) {
		LOGE << "Failed to set primary buffer format";
		return false;
	}	
	m_Initialized = true;
	//m_AllSamples.clear();
	return true;
}

// -------------------------------------------------------
// Sets primary buffer format
// -------------------------------------------------------
bool AudioManager::setPrimaryBufferFormat(DWORD dwPrimaryChannels,DWORD dwPrimaryFreq,DWORD dwPrimaryBitRate ) {
	HRESULT             hr;
	LPDIRECTSOUNDBUFFER pDSBPrimary = NULL;

	if( m_pDS == NULL )
		return false;

	// Get the primary buffer 
	DSBUFFERDESC dsbd;
	ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
	dsbd.dwSize        = sizeof(DSBUFFERDESC);
	dsbd.dwFlags       = DSBCAPS_PRIMARYBUFFER;
	dsbd.dwBufferBytes = 0;
	dsbd.lpwfxFormat   = NULL;

	if( FAILED( hr = m_pDS->CreateSoundBuffer( &dsbd, &pDSBPrimary, NULL ) ) ) {
		LOGE << "Error while creating sound buffer";
		return false;
	}

	WAVEFORMATEX wfx;
	ZeroMemory( &wfx, sizeof(WAVEFORMATEX) ); 
	wfx.wFormatTag      = (WORD) WAVE_FORMAT_PCM; 
	wfx.nChannels       = (WORD) dwPrimaryChannels; 
	wfx.nSamplesPerSec  = (DWORD) dwPrimaryFreq; 
	wfx.wBitsPerSample  = (WORD) dwPrimaryBitRate; 
	wfx.nBlockAlign     = (WORD) (wfx.wBitsPerSample / 8 * wfx.nChannels);
	wfx.nAvgBytesPerSec = (DWORD) (wfx.nSamplesPerSec * wfx.nBlockAlign);

	if( FAILED( hr = pDSBPrimary->SetFormat(&wfx) ) ) {
		LOGE << "Error while setting format";
		return false;
	}
	if (pDSBPrimary != 0) {
		delete pDSBPrimary;
	}
	return true;
}
// ------------------------------------------------
// This method will create a new AudioBuffer and
// store it in the internal map
// ------------------------------------------------
bool AudioManager::createAudioBuffer(Sound* sound) {
	LPDIRECTSOUNDBUFFER sampleHandle;
	if ( sound == NULL ) {
		LOGE <<  "No sound available";
		return false;
	}
    DSBUFFERDESC dsbd;
    ZeroMemory( &dsbd, sizeof(DSBUFFERDESC) );
	/*
    dsbd.dwSize          = sizeof(DSBUFFERDESC);
    dsbd.dwFlags         = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY; ;
    dsbd.dwBufferBytes   = sound->GetPCMBufferSize();
    dsbd.lpwfxFormat     = const_cast<WAVEFORMATEX *>(sound->GetFormat());
	HRESULT hr;
    if( FAILED( hr = m_pDS->CreateSoundBuffer( &dsbd, &sampleHandle, NULL ) ) ) {
		LOGE <<  "Failed to create soundbuffer";
        return false;
    }
	*/
	// Add handle to the list
	AudioSound as;
	//as.buffer = new AudioBuffer(sampleHandle,sound);
	as.hash = SID(sound->getName());
	//strncpy(as.name,sound->getName(),32);
	m_AudioBuffers.push_back(as);
	LOG << "adding audio sound with hash " << as.hash.get();
	//audioBuffers[soundName] = audioBuffer;	
	return true;
}

// -------------------------------------------------------
// Plays sound with given name at given volume
// -------------------------------------------------------
bool AudioManager::play(const char* soundName,int volume, bool looping) {
	AudioBuffer* buffer = findByName(soundName);
	assert(buffer != 0);
	if ( buffer != 0 ) {
		if ( buffer->isPlaying() ) {
			buffer->stop();
		}
		return buffer->play(volume,looping);
	}
	return false;
}

// -------------------------------------------------------
// Checks if a sound is already playing
// -------------------------------------------------------
bool AudioManager::isPlaying(const char* soundName) {
	AudioBuffer* buffer = findByName(soundName);
	if ( buffer != 0 ) {
		return buffer->isPlaying();
	}
	return false;
}

// -------------------------------------------------------
// Stops sound
// -------------------------------------------------------
void AudioManager::stop(const char* soundName) {
	AudioBuffer* buffer = findByName(soundName);
	if ( buffer != 0 ) {
		buffer->stop();
	}
}

// -------------------------------------------------------
// Finds sound by name
// -------------------------------------------------------
AudioBuffer* AudioManager::findByName(const char* name) {
	StaticHash hash =SID(name);
	for ( uint32_t i = 0; i < m_AudioBuffers.size();++i ) {
		if ( m_AudioBuffers[i].hash == hash ) {
			return m_AudioBuffers[i].buffer;
		}
	}
	return 0;
}

// -------------------------------------------------------
// Load sound
// -------------------------------------------------------
void AudioManager::loadSound(const char* name) {
	char fileName[256];
	sprintf(fileName,"content\\sounds\\%s.wav",name);
	Sound *s = new Sound(SID(name));
	LOG << "loading sound " << fileName;
	int ret = s->loadWavFile(fileName);
	if ( ret > 0 ) {		
		createAudioBuffer(s);		
	}
	else {
		LOGE << "Unable to load wav file";
	}
	delete s;
}

};
