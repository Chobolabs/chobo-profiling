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

#include "internal/thread.h"
#include <map>
#include <vector>
#include <string>

namespace chobo { namespace profiling
{
    class Profiler;

    class CHOBO_PROFILING_API ProfilingManager
    {
    public:
        static ProfilingManager& GetInstance();

        // returns the thread-local profiler
        Profiler& GetLocalProfiler();

        // returns a profiler by name
        // Potentially unsafe, since this profiler could be updated by a thread
        // while we're using it.
        // returns nullptr if no profiler of that name exists
        Profiler* GetProfilerByName(const std::string& name) const;
        Profiler* GetProfilerByName(const char* name) const;

        // creates a profiler which can be used with 
        // AttachNamedProfilerToCurrentThread or the PROFILE_WITH macros
        // if a profiler with such a name exists it will return it
        Profiler& CreateProfiler(const std::string& name);

        // Begin profiling for the current thread by creating a new profiler
        void BeginProfilingForCurrentThread();
        void EndProfilingForCurrentThread();

        bool IsProfilingCurrentThread();

        // Begin profiling for the current thread by creating a new profiler
        // If the name is not empty and a profiler of that name already exists, it won't create 
        // a new one, but instead reuse the existing one.
        // If the name is empty, the function behaves EXACTLY like BeginProfilingForCurrentThread
        // WARNING: if the function ends up attaching the same profiler to two different threads
        // that are already running, this WILL lead to undefined behavior (and most likely crashes)
        // MAKE SURE that if you're calling this function the threads aren't concurrent
        void AttachNamedProfilerToCurrentThread(std::string profilerName = std::string());

        typedef std::map<thread_id, Profiler*> ProfilerMap;
        const ProfilerMap& GetProfilers() const { return m_profilersPerThread; }

    private:
        ProfilingManager();
        ~ProfilingManager();
        ProfilingManager(const ProfilingManager&) = delete;
        ProfilingManager& operator=(const ProfilingManager&) = delete;

        std::map<thread_id, Profiler*> m_profilersPerThread; // may contain the same profiler per multiple ids
        std::vector<Profiler*> m_uniqueProfilers;

#if !CHOBO_PROFILER_SINGLE_THREADED
        internal::spinlock m_profilersLock;
#endif
    };


} } // namespace chobo.profiling

#if CHOBO_PROFILING_ON
#   define CHOBO_PROFILE_CURRENT_THREAD() ::chobo::profiling::ProfilingManager::GetInstance().BeginProfilingForCurrentThread()
#   define CHOBO_ATTACH_NAMED_PROFILER_TO_CURRENT_THREAD(name) ::chobo::profiling::ProfilingManager::GetInstance().AttachNamedProfilerToCurrentThread(name)

// Profiler recycling
// Sometimes you start and join a thread multiple times. This will cause the thread id to be different
// between multiple runs. On platforms with no thread-local storage this will lead to functions adding 
// profiler information to an unreachable profiler.
// For such cases you have two options. 
// 1. Either name the profiler appropriately and get it by name, also managing reports and concurrency
// 2. Attach named profilers to multiple thread ids
// In case you have thread local storate, the second will lead to multiple entries as well
// Hence this macro which does the most intuitive thing if you're recycling profilers
#   if CHOBO_PROFILER_SINGLE_THREADED || _CHOBO_HAS_THREAD_LOCAL_STORAGE
#       define CHOBO_RECYCLE_NAMED_PROFILER(name) CHOBO_PROFILE_CURRENT_THREAD()
#   else
#       define CHOBO_RECYCLE_NAMED_PROFILER(name) CHOBO_ATTACH_NAMED_PROFILER_TO_CURRENT_THREAD(name)
#   endif
#else
#   define CHOBO_PROFILE_CURRENT_THREAD()
#   define CHOBO_ATTACH_NAMED_PROFILER_TO_CURRENT_THREAD(name)
#   define CHOBO_RECYCLE_NAMED_PROFILER(name)
#endif