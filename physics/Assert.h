#pragma once
#include <Windows.h>

#ifndef XASSERT
#define XASSERT(cond, s, ...) do { \
	if (!(cond)) { \
		char buf[2048]; \
		sprintf_s(buf,2048,s, ##__VA_ARGS__); \
		LOG << "---------------------------------------------------------------";\
		LOGE << buf; \
		ds::gCrashDump->dump(); \
		LOG << "---------------------------------------------------------------";\
		MessageBoxA(NULL, buf, NULL, NULL); \
		ds::gCrashDump->exit(); \
			} \
} while (false)
#endif