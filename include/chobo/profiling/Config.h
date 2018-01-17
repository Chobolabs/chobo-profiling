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

// Main configuration file

////////////////////////////////////////////////////////////////////////////////
// Dynamic library interface

#if !defined(CHOBO_SYMBOL_IMPORT)
#   if defined(_MSC_VER)
#       define CHOBO_SYMBOL_EXPORT __declspec(dllexport)
#       define CHOBO_SYMBOL_IMPORT __declspec(dllimport)
#   else
#       define CHOBO_SYMBOL_EXPORT __attribute__((__visibility__("default")))
#       define CHOBO_SYMBOL_IMPORT
#   endif
#endif

#if defined(CHOBO_PROFILING_DYN_LINK) || defined(CHOBO_ALL_LIBS_DYN_LINK)
#   if defined(_CHOBO_PROFILING_SRC)
#       define CHOBO_PROFILING_API CHOBO_SYMBOL_EXPORT
#   else
#       define CHOBO_PROFILING_API CHOBO_SYMBOL_IMPORT
#   endif
#else
#   define CHOBO_PROFILING_API
#endif

////////////////////////////////////////////////////////////////////////////////
// Multi-threading support


#if defined(__EMSCRIPTEN__)
#   define CHOBO_PROFILER_SINGLE_THREADED 1
#else
#   define CHOBO_PROFILER_SINGLE_THREADED 0
#endif


#if CHOBO_PROFILER_SINGLE_THREADED
#   define _CHOBO_THREAD_LOCAL
#   define _CHOBO_PARTIAL_THREAD_LOCAL 0
#   define _CHOBO_HAS_THREAD_LOCAL_STORAGE 0
#else
#   if defined(_MSC_VER)
#       define _CHOBO_HAS_THREAD_LOCAL_STORAGE 1
#       if (_MSC_VER < 1900)
#           define _CHOBO_PARTIAL_THREAD_LOCAL 1
#           define _CHOBO_THREAD_LOCAL __declspec(thread)
#       else
#           define _CHOBO_PARTIAL_THREAD_LOCAL 0
#           define _CHOBO_THREAD_LOCAL thread_local
#       endif
#   elif defined(__ANDROID__)
#       define _CHOBO_HAS_THREAD_LOCAL_STORAGE 0
#   elif defined(__APPLE__)
#       include "TargetConditionals.h"
#       if(TARGET_OS_IPHONE)
#           define _CHOBO_HAS_THREAD_LOCAL_STORAGE 0
#       else
#           define _CHOBO_HAS_THREAD_LOCAL_STORAGE 1
#           define _CHOBO_PARTIAL_THREAD_LOCAL 0
#           define _CHOBO_THREAD_LOCAL thread_local
#       endif
#   else
#       define _CHOBO_HAS_THREAD_LOCAL_STORAGE 1
#       define _CHOBO_PARTIAL_THREAD_LOCAL 0
#       define _CHOBO_THREAD_LOCAL thread_local
#   endif
#endif

////////////////////////////////////////////////////////////////////////////////
// Default values

namespace chobo { namespace profiling
{
    enum ProfilingConfig
    {
        Profiler_NodePoolPageSize = 1024, // size of a node pool page in the profiler
        Profiler_NodeStaticChildren = 16, // number of preallocated children in a profiler node
    };
} } // namespace chobo.profiling

////////////////////////////////////////////////////////////////////////////////
// Misc

// silly substitute for the broken __func macro
#if defined(_MSC_VER)
#   define _CHOBO_FUNC __FUNCTION__
#else
#   define _CHOBO_FUNC __PRETTY_FUNCTION__
#endif
