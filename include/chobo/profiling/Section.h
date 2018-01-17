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
#include "chobo/small_vector.hpp"

#include <cstddef>

namespace chobo { namespace profiling
{
    class Profiler;
    class ProfilerNode;

    class CHOBO_PROFILING_API Section
    {
    public:
        Section(const char* label);
        Section(const char* label, const char* profilerName);

        size_t GetId() const { return m_id; }

        void Enter();
        void Leave();

        // leaves the current section of the current profiler
        // internally it calls leave 
        // but when debugging performs a sanity check whether the section being left is correct
        static void Leave(const char* label);

        Profiler& GetProfiler() { return m_profiler; }
        ProfilerNode& GetNode();

        // used by the split section to have the same code for both types of sections
        Section* GetLocalSection() { return this; }
    private:
        friend class Profiler;

        const char* m_label; // used for debugging purposes
        Profiler& m_profiler;
        const size_t m_id;
    };

    template <typename Sec>
    class SplitSection
    {
    public:
        SplitSection(const char* label);

        void Enter();
        void Leave();

        size_t GetId() const;
        Profiler& GetProfiler();
        ProfilerNode& GetNode();
        
    private:
        Sec m_section;
        small_vector<size_t, Profiler_NodeStaticChildren> m_subsectionIds;
    };

    template <typename Sec>
    class Tagger
    {
    public:
        Tagger(Sec& section, const char* tagName);
    };

#if ((defined(CHOBO_PROFILING_DYN_LINK) || defined(CHOBO_ALL_LIBS_DYN_LINK)))
#   if defined(_MSC_VER)
    template class CHOBO_PROFILING_API SplitSection<Section>;
    template class CHOBO_PROFILING_API Tagger<Section>;
    template class CHOBO_PROFILING_API Tagger<SplitSection<Section>>;
#   else
    extern template class CHOBO_PROFILING_API SplitSection<Section>;
    extern template class CHOBO_PROFILING_API Tagger<Section>;
    extern template class CHOBO_PROFILING_API Tagger<SplitSection<Section>>;
#   endif
#else
    extern template class SplitSection<Section>;
    extern template class Tagger<Section>;
    extern template class Tagger<SplitSection<Section>>;
#endif

#if CHOBO_PROFILER_SINGLE_THREADED || _CHOBO_HAS_THREAD_LOCAL_STORAGE
    typedef Section SectionMT;
#else
} } // namespace chobo.profiling

#include "internal/thread.h"
#include <map>

namespace chobo { namespace profiling {
    class CHOBO_PROFILING_API SectionMT
    {
    public:
        SectionMT(const char* label);
        ~SectionMT();

        size_t GetId() const;

        void Enter();
        void Leave();

        Profiler& GetProfiler();
        ProfilerNode& GetNode();

        Section* GetLocalSection();
        const Section* GetLocalSection() const;
    private:
        
        const char* m_label;

        std::map<thread_id, Section*> m_sectionsPerThreadId;

        mutable internal::spinlock m_lock;
    };

#if (defined(CHOBO_PROFILING_DYN_LINK) || defined(CHOBO_ALL_LIBS_DYN_LINK))
#   if defined(_MSC_VER)
    template class CHOBO_PROFILING_API SplitSection<SectionMT>;
    template class CHOBO_PROFILING_API Tagger<SectionMT>;
    template class CHOBO_PROFILING_API Tagger<SplitSection<SectionMT>>;
#   else
    extern template class CHOBO_PROFILING_API SplitSection<SectionMT>;
    extern template class CHOBO_PROFILING_API Tagger<SectionMT>;
    extern template class CHOBO_PROFILING_API Tagger<SplitSection<SectionMT>>;
#   endif
#else
    extern template class SplitSection<SectionMT>;
#endif
    
#endif


} } // namespace chobo.profiling
