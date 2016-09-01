#define TS_IMPLEMENTATION
#include "AudioManager.h"
#include <comdef.h>
#include "core\log\Log.h"
#include "core\string\StringUtils.h"
#include "core\io\FileRepository.h"
#include <stdint.h>
#include "tinysound.h"

// http://www.randygaul.net/2016/06/05/tinysound-release/
namespace ds {

	namespace audio {

		struct AudioContext {

			tsContext* ctx;
			Array<tsLoadedSound> sounds;

		};

		static AudioContext* audioCtx = nullptr;

		bool initialize(HWND hwnd) {
			audioCtx = new AudioContext;
			int frequency = 44000; // a good standard frequency for playing commonly saved OGG + wav files
			int latency_in_Hz = 15; // a good latency, too high will cause artifacts, too low will create noticeable delays
			int buffered_seconds = 5; // number of seconds the buffer will hold in memory. want this long enough in case of frame-delays
			int use_playing_pool = 1; // non-zero uses high-level API, 0 uses low-level API
			int num_elements_in_playing_pool = use_playing_pool ? 5 : 0; // pooled memory array size for playing sounds
			// initializes direct sound and allocate necessary memory
			audioCtx->ctx = tsMakeContext(hwnd, frequency, latency_in_Hz, buffered_seconds, num_elements_in_playing_pool);
			return true;

		}

		void shutdown() {
			if (audioCtx != nullptr) {
				tsShutdownContext(audioCtx->ctx);
				delete audioCtx;
			}			
		}

		
		int load(const StaticHash& hash) {
			int size = -1;
			char* data = repository::load(hash, &size, repository::FileType::FT_BINARY);
			tsLoadedSound airlock = tsLoadWAVRaw(data);
			delete[] data;
			if (airlock.channels != 0) {
				audioCtx->sounds.push_back(airlock);
				return audioCtx->sounds.size() - 1;
			}
			else {
				return -1;
			}
		}

		void play(int id) {
			if (id != -1) {
				tsLoadedSound& loaded = audioCtx->sounds[id];
				tsPlaySoundDef def = tsMakeDef(&loaded);
				tsPlayingSound* sound = tsPlaySound(audioCtx->ctx, def);
			}
		}

		void mix() {
			tsMix(audioCtx->ctx);
		}
	}
}
