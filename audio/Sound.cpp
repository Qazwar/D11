#include "Sound.h"
#include "core\log\Log.h"
#include "core\string\StaticHash.h"

namespace ds {

Sound::Sound(const StaticHash& name) {
	m_Name = name;
}


Sound::~Sound(void) {
	delete m_PCMBuffer;
}

bool Sound::load(const char* fileName) {
	//m_FileName = fileName;
	bool ret = true;
	FILE *file = NULL;
	file = fopen(fileName, "rb");
	if (file ==  NULL ) {
		return false;
	}
	ret = parseWave(file);
	fclose(file);
	return ret;
}

int Sound::loadWavFile(const char* filename) {
	/*
  int size=0;
  HMMIO hmfr;
  MMCKINFO parent,child;

  //init parent and child MMCKINFOs
  parent.ckid = (FOURCC)0;
  parent.cksize = 0;
  parent.fccType = (FOURCC)0;
  parent.dwDataOffset = 0;
  parent.dwFlags = 0;
  child = parent;

  //open sound file; note that mmioOpen() erroneously requires a char* even
  //    though it never changes filename 

  MMIOINFO mmioinfo;
  ZeroMemory(&mmioinfo,sizeof(MMIOINFO));
  hmfr = mmioOpenA(const_cast<char *>(filename), &mmioinfo, MMIO_READ|MMIO_ALLOCBUF);
  if( hmfr == NULL) {
	  LOGE << "Cannot open file " << filename;
	  return NULL;
  }
  //descend into the RIFF
  parent.fccType=mmioFOURCC('W','A','V','E');
  if(mmioDescend(hmfr, &parent, NULL, MMIO_FINDRIFF)){
    mmioClose(hmfr, 0); 
	//not a wave file
	LOGE << "Not a valid wav " << filename;
	return NULL;
  }

  //descend to the WAVEfmt
  child.ckid=mmioFOURCC('f', 'm', 't', ' ');
  if(mmioDescend(hmfr, &child, &parent, 0)){
    mmioClose(hmfr, 0); 
	//file has no fmt chunk
	LOGE << "file has no fmt chunk " << filename;
	return NULL;
  }

  //read the WAVEFMT from the wave file
  if(mmioRead(hmfr, (char*)&m_WavFormatEx, sizeof(m_WavFormatEx)) != sizeof(m_WavFormatEx)){
    mmioClose(hmfr, 0); 
	//unable to read fmt chunk
	LOGE << "unable to read fmt chunk " << filename;
	return NULL;
  }

  //check wave format
  if(m_WavFormatEx.wFormatTag != WAVE_FORMAT_PCM){
    mmioClose(hmfr, 0); 
	//WAVE file is not PCM format
	LOGE << "WAVE file is not PCM format " << filename;
	return NULL;
  }

  //ascend back to RIFF level
  if(mmioAscend(hmfr, &child, 0)){
    mmioClose(hmfr, 0); return NULL; //unable to ascend
  }

  //descend to the data chunk
  child.ckid=mmioFOURCC('d', 'a', 't', 'a');
  if(mmioDescend(hmfr, &child, &parent, MMIO_FINDCHUNK)){
    mmioClose(hmfr, 0); return NULL; //WAVE file has no data chunk
  }

  //grab memory to store sound
  size = child.cksize;
  m_PCMBuffer = new char[size];
  if(m_PCMBuffer == NULL){
    mmioClose(hmfr, 0); return NULL; //out of memory
  }

  //read the wave data
  if((int)mmioRead(hmfr, (char *)m_PCMBuffer, size) != size){
    //data read failed
    mmioClose(hmfr, 0); 
	delete [] m_PCMBuffer; 
	LOGE << "Data read failed " << filename;
	return NULL;
  }

  // Fill in wave properties
  m_PCMBufferSize = size;
  channels = m_WavFormatEx.nChannels;
  bits = m_WavFormatEx.wBitsPerSample;
  sampleRate = m_WavFormatEx.nSamplesPerSec;

  //close file and return
  mmioClose(hmfr, 0);
  return size;
  */
	return 0;
}

bool Sound::parseWave(FILE *fd) {
	/*
	LOG << "Parsing wave";
	DWORD file = 0; 
	DWORD fileEnd = 0; 
	
	DWORD length = 0;     
	DWORD type = 0;									

	fseek(fd, 0, SEEK_SET);

	// mmioFOURCC -- converts four chars into a 4 byte integer code.
	// The first 4 bytes of a valid .wav file is 'R','I','F','F'
	fread(&type, 1, sizeof(DWORD), fd);
	if( type != mmioFOURCC('R', 'I', 'F', 'F')) {
		LOGE <<"File does not start with RIFE - no valid file";
		return false;	
	}
	
	fread(&length, 1, sizeof(DWORD), fd); // The first integer after RIFF is the length
	fread(&type, 1, sizeof(DWORD), fd);   // The second is the block type - another code, expect.
							// 'W','A','V','E' for a legal .wav file

	if(type != mmioFOURCC('W', 'A', 'V', 'E')) {
		LOGE <<"It is not a WAV file";
		return false;		//not a WAV
	}

	// Find the end of the file
	fileEnd = length - 4;
	
	memset(&m_WavFormatEx, 0, sizeof(WAVEFORMATEX));

	m_WavFormatEx.wFormatTag        = WAVE_FORMAT_PCM; 
    m_WavFormatEx.nChannels        = 2; 
    m_WavFormatEx.nSamplesPerSec    = 44100; 
    m_WavFormatEx.wBitsPerSample    = 16; 
    m_WavFormatEx.nBlockAlign        = m_WavFormatEx.nChannels * m_WavFormatEx.wBitsPerSample / 8; 
    m_WavFormatEx.nAvgBytesPerSec    = m_WavFormatEx.nSamplesPerSec * m_WavFormatEx.nBlockAlign; 
    m_WavFormatEx.cbSize            = 0; 

	// Load the .wav format and the .wav data
	// Note that these blocks can be in either order.
	while(file < fileEnd) {
		fread(&type, 1, sizeof(DWORD), fd);   
		file += sizeof(DWORD);

		fread(&length, 1, sizeof(DWORD), fd);   
		file += sizeof(DWORD);

		switch(type)  {
			case mmioFOURCC('f', 'a', 'c', 't'):
			{
				LOGE << "This wav file is compressed.  We don't handle compressed wav at this time";
				break;
			}

			case mmioFOURCC('f', 'm', 't', ' '):
			{
				fread(&m_WavFormatEx, 1, length, fd);   
				m_WavFormatEx.cbSize = length;
				break;
			}

			case mmioFOURCC('d', 'a', 't', 'a'):
			{
				m_PCMBuffer = new char[length];
				m_PCMBufferSize = length;
				size_t bytesRead = fread(m_PCMBuffer, 1, (unsigned int)length, fd);   
				if (bytesRead < (unsigned int)length)
				{
					//assert(0 && _T("Couldn't read the sound data!"));
					return false;
				}
				break;
			}
		} 

		file += length;

		// If both blocks have been seen, we can return true.
		if( (m_PCMBuffer != 0 ) && (m_PCMBufferSize != 0) )
		{
			m_LengthMilli = ( GetPCMBufferSize() * 1000 ) / GetFormat()->nAvgBytesPerSec;
			return true;
		}

		// Increment the pointer past the block we just read,
		// and make sure the pointer is byte aliged.
		if (length & 1)
		{
			char temp;
			fread(&temp, 1, 1, fd);   
			file ++;
		}
	} 

	// If we get to here, the .wav file didn't contain all the right pieces.
	return false; 
	*/
	return true;
}

};
