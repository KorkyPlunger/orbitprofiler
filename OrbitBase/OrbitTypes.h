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
#include <unistd.h>

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
#define __int8 char
#define LONG long
typedef DWORD ULONG;
struct TypeInfo{};
#define HMODULE void*
inline void SetThreadName(int, const char*){}
#define FILETIME ULONG64
#define FLT_MAX __FLT_MAX__
#define TEXT( x ) L##x
#define USHORT unsigned short
#define UCHAR unsigned char
inline void Sleep(int millis){ usleep( (float)millis*1000.f ); }


#define GetCurrentThreadId pthread_self


struct PerfCounter
{
    void start(){};
    void stop(){};
};

#endif
