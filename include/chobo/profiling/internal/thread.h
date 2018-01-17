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

// File to include in place of <thread>
// Contains chobo-profiling-specific stuff

#include "../Config.h"

#if CHOBO_PROFILER_SINGLE_THREADED
namespace chobo
{
    typedef int thread_id;
    inline thread_id GetCurrentThreadId() { return 0; }
}
#else
#include <thread>
#include <atomic>

namespace internal
{
    class spinlock
    {
    public:
        spinlock()
        {
            locked.clear();
        }

        void lock()
        {
            while (locked.test_and_set(std::memory_order_acquire)) { ; }
        }
        void unlock()
        {
            locked.clear(std::memory_order_release);
        }
    private:
        std::atomic_flag locked;
    };

    class spinlock_sentry
    {
    public:
        spinlock_sentry(spinlock& l)
            : lock(l)
        {
            l.lock();
        }

        ~spinlock_sentry()
        {
            lock.unlock();
        }

    private:
        spinlock& lock;
    };
}

namespace chobo
{
    typedef ::std::thread::id thread_id;
    inline thread_id GetCurrentThreadId() { return std::this_thread::get_id(); }
}

#endif