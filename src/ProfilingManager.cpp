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
#include <chobo/profiling/ProfilingManager.h>
#include <chobo/profiling/Profiler.h>

namespace chobo { namespace profiling
{
#if _CHOBO_HAS_THREAD_LOCAL_STORAGE
    _CHOBO_THREAD_LOCAL
#endif
        Profiler* localProfiler;

    ProfilingManager& ProfilingManager::GetInstance()
    {
        static ProfilingManager instance;
        return instance;
    }

    ProfilingManager::ProfilingManager()
    {
    }

    ProfilingManager::~ProfilingManager()
    {
        for (auto& p : m_uniqueProfilers)
        {
            delete p;
        }
    }

    bool ProfilingManager::IsProfilingCurrentThread()
    {
#if CHOBO_PROFILER_SINGLE_THREADED || _CHOBO_HAS_THREAD_LOCAL_STORAGE
        return !!localProfiler;
#else
        thread_id id = GetCurrentThreadId();

        internal::spinlock_sentry sentry(m_profilersLock);
        auto p = m_profilersPerThread.find(id);
        return p != m_profilersPerThread.end();
#endif
    }

    Profiler& ProfilingManager::GetLocalProfiler()
    {   
#if CHOBO_PROFILER_SINGLE_THREADED || _CHOBO_HAS_THREAD_LOCAL_STORAGE
        return *localProfiler;
#else
        thread_id id = GetCurrentThreadId();

        internal::spinlock_sentry sentry(m_profilersLock);
        auto p = m_profilersPerThread.find(id);
        assert(p != m_profilersPerThread.end());
        return *p->second;
#endif
    }

    void ProfilingManager::BeginProfilingForCurrentThread()
    {
        AttachNamedProfilerToCurrentThread(std::string());
    }

    void ProfilingManager::AttachNamedProfilerToCurrentThread(std::string profilerName)
    {
        thread_id id = GetCurrentThreadId();

#if !CHOBO_PROFILER_SINGLE_THREADED
        internal::spinlock_sentry sentry(m_profilersLock);
#endif

        auto p = m_profilersPerThread.find(id);
        
        // if a profiler exists for this thread id and it has the same name as the required one, go ahead and reuse it
        if (p != m_profilersPerThread.end() && (profilerName.empty() || p->second->GetName() == profilerName))
        {
            localProfiler = p->second;
        }
        // otherwise we either requested a special named profiler (existing at some other thread id)
        // or are beginning profiling for the first time
        // In any case the behavor is the same
        else
        {
            if (profilerName.empty())
            {
                localProfiler = new Profiler;
                m_profilersPerThread[id] = localProfiler;
                m_uniqueProfilers.push_back(localProfiler);
            }
            else
            {
                auto p = GetProfilerByName(profilerName);

                if (p)
                {
                    localProfiler = p;
                    m_profilersPerThread[id] = localProfiler;
                }
                else
                {
                    // no profiler of the same name has been found
                    // create a new one
                    localProfiler = new Profiler;
                    m_profilersPerThread[id] = localProfiler;
                    m_uniqueProfilers.push_back(localProfiler);
                    localProfiler->SetName(profilerName);
                }
            }
        }
    }

    Profiler* ProfilingManager::GetProfilerByName(const std::string& name) const
    {
        return GetProfilerByName(name.c_str());
    }

    Profiler* ProfilingManager::GetProfilerByName(const char* name) const
    {
        for (auto& p : m_uniqueProfilers)
        {
            if (p->GetName() == name)
            {
                return p;
            }
        }

        return nullptr;
    }

    Profiler& ProfilingManager::CreateProfiler(const std::string& name)
    {
        auto profiler = GetProfilerByName(name);
        if (profiler) return *profiler;

        profiler = new Profiler;
        profiler->SetName(name);

#if !CHOBO_PROFILER_SINGLE_THREADED
        internal::spinlock_sentry sentry(m_profilersLock);
#endif
        m_uniqueProfilers.push_back(profiler);
        return *profiler;
    }

    void ProfilingManager::EndProfilingForCurrentThread()
    {
        // nothing to do for now
    }

} } // namespace chobo.profiling
