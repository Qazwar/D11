#include "system_info.h"
#include "BaseApp.h"

namespace ds {

	namespace sysinfo {

		void getProcessor(SystemInfo* info) {
			HKEY pHRoot;
			LONG res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System", 0, KEY_READ, &pHRoot);
			if (res != ERROR_SUCCESS) {
				strncpy(info->processor, "Unknown", 8);
				RegCloseKey(pHRoot);
				return;
			}

			HKEY key;
			if (RegOpenKey(pHRoot, "CentralProcessor\\0", &key) != ERROR_SUCCESS) {
				strncpy(info->processor, "Unknown", 8);
				RegCloseKey(pHRoot);
				return;
			}

			unsigned char data[1024];
			unsigned long datasize = 1024;
			DWORD type;

			if (RegQueryValueEx(key, "ProcessorNameString", 0, &type, data, &datasize) != ERROR_SUCCESS) {
				strncpy(info->processor, "Unknown", 8);
			}
			else if (type == REG_SZ) {
				data[datasize] = 0;

				strncpy(info->processor, (char*)(data + 0), datasize);
				info->processor[datasize] = 0;
			}


			if (RegQueryValueEx(key, "~MHz", 0, &type, data, &datasize) != ERROR_SUCCESS) {
				info->mhz = 13;
			}
			else if (type == REG_DWORD) {
				info->mhz = *((int*)data);
			}
			RegCloseKey(key);
			RegCloseKey(pHRoot);

		}

		void getGFX(SystemInfo* info) {
			DISPLAY_DEVICE dd;
			dd.cb = sizeof(DISPLAY_DEVICE);
			int i = 0;

			while (EnumDisplayDevices(NULL, i, &dd, 0)) {
				if (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) {
					strncpy(info->gpuModel, dd.DeviceString, 64);
				}
				++i;
			}
		}

		void getRAMInformation(SystemInfo* info) {
			MEMORYSTATUSEX ms;
			ms.dwLength = sizeof(ms);
			GlobalMemoryStatusEx(&ms);

			info->free_memory_MB = ms.ullAvailPhys;
			info->total_memory_MB = ms.ullTotalPhys;
			for (int i = 0; i < 2; ++i) {
				info->free_memory_MB /= 1024;
				info->total_memory_MB /= 1024;
			}
		}

	}
}