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

#include <cstddef>
#include <memory>

namespace chobo { namespace profiling {

class CHOBO_PROFILING_API MemoryProfilingScope
{
public:
    MemoryProfilingScope();
    ~MemoryProfilingScope();
};

extern CHOBO_PROFILING_API void _profile_alloc(size_t size);
extern CHOBO_PROFILING_API void _profile_dealloc();

#if CHOBO_PROFILING_ON && !defined(CHOBO_MEMORY_PROFILING_OFF)
#define CHOBO_PROFILE_ALLOC(size) ::chobo::profiling::_profile_alloc(size)
#define CHOBO_PROFILE_DEALLOC() ::chobo::profiling::_profile_dealloc()
#define CHOBO_MEMORY_PROFILE_SCOPE() ::chobo::profiling::MemoryProfilingScope __memoryProfilingScope;
#else
#define CHOBO_PROFILE_ALLOC(size)
#define CHOBO_PROFILE_DEALLOC()
#define CHOBO_MEMORY_PROFILE_SCOPE()
#endif

} } // namespace chobo.profiling