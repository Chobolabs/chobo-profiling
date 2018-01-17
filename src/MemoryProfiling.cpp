//
// chobo-profiling
// Copyright (c) 2015-2042 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#include "Internal.h"
#include <chobo/profiling/MemoryProfiling.h>

#include <chobo/profiling/ProfilingManager.h>
#include <chobo/profiling/Profiler.h>
#include <chobo/profiling/ProfilerNode.h>


#if CHOBO_PROFILER_SINGLE_THREADED
#       define Profiler_profiler() static Profiler& profiler = ProfilingManager::GetInstance().GetLocalProfiler()
#else
#   if _CHOBO_HAS_THREAD_LOCAL_STORAGE
#       if _CHOBO_PARTIAL_THREAD_LOCAL
#           define Profiler_profiler() \
                _CHOBO_THREAD_LOCAL static Profiler* pProfiler = nullptr; \
                if (!pProfiler) \
                { \
                    pProfiler = &ProfilingManager::GetInstance().GetLocalProfiler(); \
                } \
                Profiler& profiler = *pProfiler
#       else
#           define Profiler_profiler() _CHOBO_THREAD_LOCAL static Profiler& profiler = ProfilingManager::GetInstance().GetLocalProfiler()
#       endif
#   else
#       define Profiler_profiler() Profiler& profiler = ProfilingManager::GetInstance().GetLocalProfiler()
#   endif
#endif

#if _CHOBO_HAS_THREAD_LOCAL_STORAGE

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace chobo
{
namespace profiling
{


static std::atomic<int> ProfilingScope_StackLevel;

static _CRT_ALLOC_HOOK AllocHook_Old;

static int ProfilingAllocHook(int nAllocType, void *pvData,
    size_t nSize, int nBlockUse, long lRequest,
    const unsigned char * szFileName, int nLine)
{
    if (ProfilingManager::GetInstance().IsProfilingCurrentThread())
    {
        switch (nAllocType)
        {
        case _HOOK_ALLOC:
        case _HOOK_REALLOC:
            _profile_alloc(nSize);
            break;
        case _HOOK_FREE:
            _profile_dealloc();
            break;
        default:
            assert(false); // wut?
        }
    }

    if (AllocHook_Old)
    {
        return AllocHook_Old(nAllocType, pvData, nSize, nBlockUse, lRequest, szFileName, nLine);
    }
    return TRUE;
}

MemoryProfilingScope::MemoryProfilingScope()
{
    if (ProfilingScope_StackLevel == 0)
    {
        AllocHook_Old = _CrtSetAllocHook(ProfilingAllocHook);
    }
    ++ProfilingScope_StackLevel;
}

MemoryProfilingScope::~MemoryProfilingScope()
{
    if (ProfilingScope_StackLevel == 1)
    {
        auto prevHook = _CrtSetAllocHook(AllocHook_Old);
        assert(prevHook == ProfilingAllocHook); // uh-oh someone else is hooking
    }
    assert(ProfilingScope_StackLevel > 0);
    --ProfilingScope_StackLevel;
}

}
}

#elif defined(__APPLE__) && !defined(CHOBO_PLATFORM_IOS)

#include <malloc/malloc.h>
#include <unistd.h>
#include <sys/mman.h>

static std::atomic<int> ProfilingScope_StackLevel;
static malloc_zone_t* Default_Zone;
static malloc_zone_t Backup_Zone;

namespace chobo
{
namespace profiling
{

static void AttachHooks();
static void DetachHooks();

static void* MallocHook(_malloc_zone_t *zone, size_t size)
{
    DetachHooks();

    if (ProfilingManager::GetInstance().IsProfilingCurrentThread())
    {
        _profile_alloc(size);
    }

    AttachHooks();
    return Backup_Zone.malloc(zone, size);
}

static void FreeHook(_malloc_zone_t *zone, void *ptr)
{
    DetachHooks();

    if (ProfilingManager::GetInstance().IsProfilingCurrentThread())
    {
        _profile_dealloc();
    }

    AttachHooks();
    return Backup_Zone.free(zone, ptr);
}

static void AttachHooks()
{
    vm_address_t pageStart = reinterpret_cast<vm_address_t>(Default_Zone)
                           & static_cast<vm_size_t>(~(getpagesize() - 1));

    vm_size_t len = reinterpret_cast<vm_address_t>(Default_Zone)
                  - pageStart + sizeof(malloc_zone_t);

    mprotect(reinterpret_cast<void*>(pageStart), len, PROT_READ | PROT_WRITE);

    Default_Zone->malloc = MallocHook;
    Default_Zone->free = FreeHook;
}

static void DetachHooks()
{
    *Default_Zone = Backup_Zone;
}

MemoryProfilingScope::MemoryProfilingScope()
{
    if (!Default_Zone)
    {
        Default_Zone = malloc_default_zone();
        Backup_Zone = *Default_Zone;
    }

    if (ProfilingScope_StackLevel == 0)
    {
        AttachHooks();
    }
    ++ProfilingScope_StackLevel;
}

MemoryProfilingScope::~MemoryProfilingScope()
{
    if (ProfilingScope_StackLevel == 1)
    {
        DetachHooks();
    }
    assert(ProfilingScope_StackLevel > 0);
    --ProfilingScope_StackLevel;
}
}
}

#else

namespace chobo
{
    namespace profiling
    {
        MemoryProfilingScope::MemoryProfilingScope()
        {
        }

        MemoryProfilingScope::~MemoryProfilingScope()
        {
        }
    }
}

#endif

#else // not _CHOBO_HAS_THREAD_LOCAL_STORAGE

// mobile device... noting to do
namespace chobo
{
namespace profiling
{
MemoryProfilingScope::MemoryProfilingScope()
{
}

MemoryProfilingScope::~MemoryProfilingScope()
{
}
}
}

#endif // _CHOBO_HAS_THREAD_LOCAL_STORAGE

namespace chobo
{
namespace profiling
{
void _profile_alloc(size_t size)
{
    Profiler_profiler();
    profiler.ProfileAlloc(size);
}

void _profile_dealloc()
{
    Profiler_profiler();
    profiler.ProfileDealloc();
}

}
}
