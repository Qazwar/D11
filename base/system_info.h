#pragma once

namespace ds {

	struct SystemInfo {
		unsigned int free_memory_MB;
		unsigned int total_memory_MB;
		int number_cpus;
		char processor[512];
		int mhz;
		char gpuVendor[64];
		char gpuModel[512];
		int screenResolution[2];
		int numMonitors;
	};

	namespace sysinfo {

		void getProcessor(SystemInfo* info);

		void getGFX(SystemInfo* info);

		void getRAMInformation(SystemInfo* info);

	}
}