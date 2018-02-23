//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>

#pragma pack(push, 8)

namespace Orbit {

//-----------------------------------------------------------------------------
struct Data
{
    Data() { std::memset(this, 0, sizeof(*this)); }
    ULONG64       m_Time;
    ULONG64       m_CallstackHash;
    unsigned long m_ThreadId;
    int           m_NumBytes;
    void*         m_Data;
};

}

#pragma pack(pop)
