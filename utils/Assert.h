#pragma once
#include <Windows.h>
#include "..\base\CrashReporter.h"
#include "Log.h"

#ifndef XASSERT
//#define XASSERT(Expr, s) do { MyAssert(#Expr, Expr,__FILE__,__LINE__,s); } while(false);
#define XASSERT(Expr, s, ...) do { MyAssert_fmt(#Expr, Expr,__FILE__,__LINE__,s,__VA_ARGS__); } while(false);
#endif

