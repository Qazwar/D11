#pragma once
#include <Windows.h>
#include "..\base\CrashReporter.h"

#ifndef XASSERT
#define XASSERT(cond, s, ...) do { \
	if (!(cond)) { \
		char buf[2048]; \
		sprintf_s(buf,2048,s, ##__VA_ARGS__); \
		LOG << "---------------------------------------------------------------";\
		LOGE << buf; \
		LOG << "---------------------------------------------------------------";\
		MessageBoxA(NULL, buf, NULL, NULL); \
		assert(cond); \
		} \
} while (false)
#endif