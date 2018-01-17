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

#include "profiling/Config.h"

// Main include file

#if CHOBO_PROFILING_ON

#   include "profiling/Section.h"
#   include "profiling/ProfileSentry.h"
#   include <type_traits>

#   if CHOBO_PROFILER_SINGLE_THREADED
#       define CHOBO_PROFILING_SECTION(Label) \
            static ::chobo::profiling::Section __choboProfilingSection(Label);
#       define CHOBO_PROFILING_SECTION_MT CHOBO_PROFILING_SECTION
#       define CHOBO_PROFILING_SPLIT_SECTION(Label) CHOBO_PROFILING_SECTION
#       define CHOBO_PROFILING_SPLIT_SECTION_MT(Label) CHOBO_PROFILING_SECTION
#   else
#       if _CHOBO_HAS_THREAD_LOCAL_STORAGE
#           if _CHOBO_PARTIAL_THREAD_LOCAL
#               define CHOBO_PROFILING_SECTION(Label) \
                    _CHOBO_THREAD_LOCAL static ::chobo::profiling::Section* __pChoboProfilingSection = nullptr; \
                    if(!__pChoboProfilingSection) __pChoboProfilingSection = new ::chobo::profiling::Section(Label); \
                    auto& __choboProfilingSection = *__pChoboProfilingSection;
#               define CHOBO_PROFILING_SPLIT_SECTION(Label) \
                    _CHOBO_THREAD_LOCAL static ::chobo::profiling::SplitSection<::chobo::profiling::Section>* __pChoboProfilingSection = nullptr; \
                    if(!__pChoboProfilingSection) __pChoboProfilingSection = new ::chobo::profiling::SplitSection<::chobo::profiling::Section>(Label); \
                    auto& __choboProfilingSection = *__pChoboProfilingSection;
#           else
#               define CHOBO_PROFILING_SECTION(Label) \
                    _CHOBO_THREAD_LOCAL static ::chobo::profiling::Section __choboProfilingSection(Label);
#               define CHOBO_PROFILING_SPLIT_SECTION(Label) \
                    _CHOBO_THREAD_LOCAL static ::chobo::profiling::SplitSection<::chobo::profiling::Section> __choboProfilingSection(Label);
#           endif
#           define CHOBO_PROFILING_SECTION_MT CHOBO_PROFILING_SECTION
#           define CHOBO_PROFILING_SPLIT_SECTION_MT CHOBO_PROFILING_SPLIT_SECTION
#       else
#           define CHOBO_PROFILING_SECTION(Label) \
                static ::chobo::profiling::Section __choboProfilingSection(Label);
#           define CHOBO_PROFILING_SECTION_MT(Label) \
                static ::chobo::profiling::SectionMT __choboProfilingSection(Label);
#           define CHOBO_PROFILING_SPLIT_SECTION(Label) \
                static ::chobo::profiling::SplitSection<::chobo::profiling::Section> __choboProfilingSection(Label);
#           define CHOBO_PROFILING_SPLIT_SECTION_MT(Label) \
                static ::chobo::profiling::SplitSection<::chobo::profiling::SectionMT> __choboProfilingSection(Label);
#       endif
#   endif

// profiling of a scope with a label
#   define CHOBO_PROFILE_SCOPE(Label) \
    CHOBO_PROFILING_SECTION(Label); \
    ::chobo::profiling::ProfileSentry<::chobo::profiling::Section> __choboSentry(__choboProfilingSection)

#   define CHOBO_PROFILE_SCOPE_MT(Label) \
    CHOBO_PROFILING_SECTION_MT(Label); \
    ::chobo::profiling::ProfileSentry<::chobo::profiling::SectionMT> __choboSentry(__choboProfilingSection)

// each entry will be added as a separate node
#   define CHOBO_PROFILE_SCOPE_SPLIT(Label) \
    CHOBO_PROFILING_SPLIT_SECTION(Label); \
    ::chobo::profiling::ProfileSentry<::chobo::profiling::SplitSection<::chobo::profiling::Section>> __choboSentry(__choboProfilingSection)

#   define CHOBO_PROFILE_SCOPE_SPLIT_MT(Label) \
    CHOBO_PROFILING_SPLIT_SECTION_MT(Label); \
    ::chobo::profiling::ProfileSentry<::chobo::profiling::SplitSection<::chobo::profiling::SectionMT>> __choboSentry(__choboProfilingSection)

// profiling a function
#   define CHOBO_PROFILE_FUNC() CHOBO_PROFILE_SCOPE(_CHOBO_FUNC)

#   define CHOBO_PROFILE_FUNC_MT() CHOBO_PROFILE_SCOPE_MT(_CHOBO_FUNC)

// each entry will be added as a separate node
#   define CHOBO_PROFILE_FUNC_SPLIT() CHOBO_PROFILE_SCOPE_SPLIT(_CHOBO_FUNC)

#   define CHOBO_PROFILE_FUNC_SPLIT_MT() CHOBO_PROFILE_SCOPE_SPLIT_MT(_CHOBO_FUNC)

// profiling with manual enter and leave
#   define CHOBO_PROFILE_SECTION_ENTER(Label) \
    CHOBO_PROFILING_SECTION(Label); \
    __choboProfilingSection.Enter()

#   define CHOBO_PROFILE_SECTION_LEAVE(Label) \
    ::chobo::profiling::Section::Leave(Label)

// profiling with a specified profiler name
// the profiler with that name must be created before using it by Manager::CreateProfiler or RecycleNamedProfiler
// WARNING: profiling with a specified profiler name in different threads will lead to undefined behavior
#define CHOBO_PROFILE_SCOPE_WITH(Label, ProfilerName) \
    static ::chobo::profiling::Section __choboProfilingSection(Label, ProfilerName); \
    ::chobo::profiling::ProfileSentry<::chobo::profiling::Section> __choboSentry(__choboProfilingSection)

#define CHOBO_PROFILE_FUNC_WITH(ProfilerName) CHOBO_PROFILE_SCOPE_WITH(_CHOBO_FUNC, ProfilerName)

#define CHOBO_PROFILE_TAG(TagName) \
    static ::chobo::profiling::Tagger<std::remove_reference<decltype(__choboProfilingSection)>::type> \
        __choboProfilingTagger(__choboProfilingSection, TagName);


#else

#   define CHOBO_PROFILE_SCOPE(Label)
#   define CHOBO_PROFILE_SCOPE_MT(Label)
#   define CHOBO_PROFILE_SCOPE_SPLIT_MT(Label)
#   define CHOBO_PROFILE_SCOPE_SPLIT(Label)
#   define CHOBO_PROFILE_FUNC()
#   define CHOBO_PROFILE_FUNC_MT()
#   define CHOBO_PROFILE_FUNC_SPLIT()
#   define CHOBO_PROFILE_FUNC_SPLIT_MT()
#   define CHOBO_PROFILE_SECTION_ENTER(Label)
#   define CHOBO_PROFILE_SECTION_LEAVE(Label)
#   define CHOBO_PROFILE_SCOPE_WITH(Label, ProfilerName)
#   define CHOBO_PROFILE_FUNC_WITH(ProfilerName)
#   define CHOBO_PROFILE_TAG(TagName)

#endif

#include "profiling/MemoryProfiling.h"
