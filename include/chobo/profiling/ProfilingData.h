//
// chobo-profiling
// Copyright (c) 2015-2018 Chobolabs Inc. 
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#pragma once

#include "Config.h"

#include <cstdint>
#include <cstddef>

namespace chobo { namespace profiling
{
    struct CHOBO_PROFILING_API ProfilingData
    {
        // mean time spent in this node
        // this on each call so it might be a tiny bit slower than the simple getters
        uint64_t GetMeanTime() const { return time / timesEntered; }

        // Clear the profiling data
        // Doesn't touch the other data
        void Clear()
        {
            time = 0;
            timesEntered = 0;
            allocations = 0;
            allocatedMemory = 0;
            deallocations = 0;
        }

        // merges the profiling data of ot two instances into this one
        // call stack level and extra data from "other" are ignored
        void Append(const ProfilingData& other)
        {
            time += other.time;
            timesEntered += other.timesEntered;
            allocations += other.allocations;
            allocatedMemory += other.allocatedMemory;
            deallocations += other.deallocations;
        }

        ///////////////////////////////////////////////////////////////////////
        // Performance profiling data
        uint64_t time = 0; // total time spent in this node in nanoseconds
        unsigned timesEntered = 0; // number of times this node has been entered

        ///////////////////////////////////////////////////////////////////////
        // Memory profiling data
        unsigned allocations = 0; // number of allocations in the node
        size_t allocatedMemory = 0; // allocated memory in the node
        unsigned deallocations = 0; // number of deallocations in the node

        ///////////////////////////////////////////////////////////////////////
        // Other data

        // level in the call stack of this node
        int callStackLevel = 0;

        // node extra data
        // use it to associate some data of your own for the node
        intptr_t extraData = 0;
    };
} } // namespace chobo.profiling
