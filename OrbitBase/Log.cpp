//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Log.h"

Logger GLogger;

Logger & Logger::GetLogger()
{
    return GLogger;
}
