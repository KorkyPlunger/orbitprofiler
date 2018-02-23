//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#ifdef _WIN32
#include <winstl/performance/performance_counter.hpp>

typedef winstl::performance_counter                 PerfCounter;
typedef winstl::performance_counter::interval_type  IntervalType;
typedef winstl::performance_counter::epoch_type     EpochType;
#else

#include "OrbitTypes.h"

struct PerfCounter
{
    void start(){};
    void stop(){};
    IntervalType get_start() const { return 0; }
    IntervalType get_end()   const { return 0; }
    void set_start( IntervalType start ){}
    void set_end( IntervalType end ){}
    static IntervalType get_microseconds( IntervalType a, IntervalType b ){ return 0; }
    static IntervalType get_period_count_from_microseconds( IntervalType a ){ return 0; }
};

#endif
