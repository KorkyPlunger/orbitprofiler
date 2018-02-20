//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#ifdef _WIN32
#include <wtypes.h>
#define _VSNWPRINTF vsnwprintf_s
#endif

#ifdef __linux__
#include <inttypes.h>
#include <limits.h>

#define WCHAR wchar_t
#define VSNWPRINTF(a, b, c, d)
#define GUID unsigned long long
#define ULONG64 unsigned long long
#define DWORD64 ULONG64
typedef unsigned long DWORD;
#define IntervalType DWORD64
#define __int64 int64_t
#define _mkdir( x )
#define TCHAR wchar_t
#define MAX_PATH PATH_MAX
#define HANDLE void*


#define GetCurrentThreadId pthread_self


struct PerfCounter
{
    void start(){};
    void stop(){};
};

#endif
