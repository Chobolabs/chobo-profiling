//
// chobo-profiling
// Copyright (c) 2015-2018 Chobolabs Inc. 
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#include "../Internal.h"
#include <chobo/profiling/internal/high_res_clock.h>
#include <thread>

#if !defined(CHOBO_PROFILING_TEST)
#if defined(_MSC_VER) || defined(__MINGW32__)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace chobo
{
    const long long FREQ = []() -> long long
    {
        LARGE_INTEGER frequency;
        QueryPerformanceFrequency(&frequency);
        return frequency.QuadPart;
    }();

    high_res_clock::time_point high_res_clock::now()
    {
        LARGE_INTEGER t;
        QueryPerformanceCounter(&t);
        return time_point(duration((t.QuadPart * rep(period::den))/FREQ));
    }
}

#endif
#else

namespace
{
struct Time
{
    uint64_t now;
};

Time theTime;
}

namespace chobo
{

high_res_clock::time_point high_res_clock::now()
{
    auto ret = time_point(duration(theTime.now));
    return ret;
}

namespace test
{
void this_thread_sleep_for_ns(uint64_t ns)
{
    theTime.now += ns;
}
}
}
#endif

