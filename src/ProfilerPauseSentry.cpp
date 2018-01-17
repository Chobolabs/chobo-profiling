//
// chobo-profiling
// Copyright (c) 2015-2018 Chobolabs Inc. 
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#include "Internal.h"
#include <chobo/profiling/ProfilerPauseSentry.h>
#include <chobo/profiling/Profiler.h>
#include <chobo/profiling/ProfilingManager.h>

namespace chobo { namespace profiling
{

    ProfilerPauseSentry::ProfilerPauseSentry(Profiler& profiler)
        : m_profiler(profiler)
    {
        m_profiler.Pause();
    }

    ProfilerPauseSentry::ProfilerPauseSentry()
        : ProfilerPauseSentry(ProfilingManager::GetInstance().GetLocalProfiler())
    {
    }

    ProfilerPauseSentry::~ProfilerPauseSentry()
    {
        m_profiler.Resume();
    }
} } // namespace chobo.profiling
