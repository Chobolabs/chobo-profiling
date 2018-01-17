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

// MSVC doesn't support a the high resolution timer from chrono
// (well it does, but it's "high" resolution is about a millisecond)
// Thus we're forced to use this silly class when we're on msvc

#include "../Config.h"
#include <chrono>
#include <cstdint>

namespace chobo
{
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(CHOBO_PROFILING_TEST)
struct CHOBO_PROFILING_API high_res_clock
{
    typedef long long rep;
    typedef std::nano period;
    typedef std::chrono::duration<rep, period> duration;
    typedef std::chrono::time_point<high_res_clock> time_point;
    static const bool is_steady = true;

    static time_point now();
};
#else
typedef std::chrono::high_resolution_clock high_res_clock;
#endif

#if defined(CHOBO_PROFILING_TEST)

namespace test
{

CHOBO_PROFILING_API void this_thread_sleep_for_ns(uint64_t ns);

template< class Rep, class Period >
void this_thread_sleep_for(const std::chrono::duration<Rep, Period>& duration)
{
    this_thread_sleep_for_ns(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count());
}

}
#endif
}