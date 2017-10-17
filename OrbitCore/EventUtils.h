//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define INITGUID
#include <evntrace.h>
#include <evntcons.h>

//struct _EVENT_RECORD;
//typedef _EVENT_RECORD EVENT_RECORD;
//typedef EVENT_RECORD* PEVENT_RECORD;

namespace EventUtils
{
    void OutputDebugEvent( PEVENT_RECORD pEvent );
}