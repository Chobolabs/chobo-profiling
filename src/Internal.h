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

// Definitions for the chobo-profiling source files only

#define _CHOBO_PROFILING_SRC

// You don't need to define this unless you debug the profiler itself
#define CHOBO_PROFILER_DEBUG 1

#include <cassert>

#if CHOBO_PROFILER_DEBUG
#   define CHOBO_PROFILER_ASSERT(x) assert(x)
#else
#   define CHOBO_PROFILER_ASSERT(x)
#endif
