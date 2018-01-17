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

namespace chobo { namespace profiling
{
    class Profiler;

    class CHOBO_PROFILING_API ProfilerPauseSentry
    {
    public:
        ProfilerPauseSentry(Profiler& profiler);
        ProfilerPauseSentry();
        ~ProfilerPauseSentry();

    private:
        Profiler& m_profiler;
    };

} } // namespace chobo.profiling

#define CHOBO_PROFILER_PAUSE_SENTRY(profiler) ::chobo::profiling::ProfilerPauseSentry __choboProfilerPauseSentry(profiler)
#define CHOBO_LOCAL_PROFILER_PAUSE_SENTRY() ::chobo::profiling::ProfilerPauseSentry __choboProfilerPauseSentry
