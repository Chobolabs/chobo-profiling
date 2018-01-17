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

#include <chobo/profiling/Section.h>
#include <chobo/profiling/ProfilingManager.h>
#include <chobo/profiling/Profiler.h>


namespace chobo { namespace profiling
{
    Section::Section(const char* label)
        : m_label(label)
        , m_profiler(ProfilingManager::GetInstance().GetLocalProfiler())
        , m_id(m_profiler.AddSection(label))
    {}

    Section::Section(const char* label, const char* profilerName)
        : m_label(label)
        , m_profiler(*ProfilingManager::GetInstance().GetProfilerByName(profilerName))
        , m_id(m_profiler.AddSection(label))
    {
        assert(ProfilingManager::GetInstance().GetProfilerByName(profilerName));
    }

    void Section::Enter()
    {
        m_profiler.Enter(m_id);
    }

    void Section::Leave()
    {
        m_profiler.Leave();
    }

    void Section::Leave(const char* label)
    {
        auto& profiler = ProfilingManager::GetInstance().GetLocalProfiler();
        profiler.Leave(label);
    }

    ProfilerNode& Section::GetNode()
    {
        return m_profiler.GetCurrentNode();
    }

    ///////////////////////////////////////////////////////////////////////////
    
    namespace
    {
    const char* SplitSubsectionIds[] = {
        "01", "02", "03", "04",
        "05", "06", "07", "08",
        "09", "10", "11", "12",
        "13", "14", "15", "16",
    };
    const unsigned Num_SplitSubsectionIds = sizeof(SplitSubsectionIds) / sizeof(const char*);
    }

    template <typename Sec>
    SplitSection<Sec>::SplitSection(const char* label)
        : m_section(label)
    {
    }

    template <typename Sec>
    void SplitSection<Sec>::Enter()
    {
        Section* section = m_section.GetLocalSection();
        section->Enter();
        auto& profiler = section->GetProfiler();
        auto& node = profiler.GetCurrentNode();
        auto te = node.GetProfilingData().timesEntered - 1;
        if (te >= m_subsectionIds.size())
        {
            m_subsectionIds.reserve(te + 1);
            for (auto i = m_subsectionIds.size(); i <= te; ++i)
            {
                m_subsectionIds.push_back(profiler.AddSection(SplitSubsectionIds[i % Num_SplitSubsectionIds]));
            }
        }
        profiler.Enter(m_subsectionIds[te]);
    }

    template <typename Sec>
    void SplitSection<Sec>::Leave()
    {
        Section* section = m_section.GetLocalSection();
        section->GetProfiler().Leave();
        section->Leave();
    }
    
    template <typename Sec>
    Profiler& SplitSection<Sec>::GetProfiler()
    {
        return m_section.GetProfiler();
    }

    template <typename Sec>
    ProfilerNode& SplitSection<Sec>::GetNode()
    {
        return m_section.GetNode();
    }

    template <typename Sec>
    size_t SplitSection<Sec>::GetId() const
    {
        return m_section.GetId();
    }

    ///////////////////////////////////////////////////////////////////////////

    template <typename Sec>
    Tagger<Sec>::Tagger(Sec& section, const char* tagName)
    {
        // manually add first node
        auto& node = section.GetNode();
        auto& profiler = section.GetProfiler();
        auto& tag = profiler.GetTag(section.GetId(), tagName);

        node.SetTag(&tag);
    }

    ///////////////////////////////////////////////////////////////////////////

#if !CHOBO_PROFILER_SINGLE_THREADED && !_CHOBO_HAS_THREAD_LOCAL_STORAGE
    SectionMT::SectionMT(const char* label)
        : m_label(label)
    {
    }

    SectionMT::~SectionMT()
    {
        for (auto& s : m_sectionsPerThreadId)
        {
            delete s.second;
        }
    }

    void SectionMT::Enter()
    {
        auto s = GetLocalSection();
        s->Enter();
    }

    void SectionMT::Leave()
    {
        auto s = GetLocalSection();
        s->Leave();
    }

    Profiler& SectionMT::GetProfiler()
    {
        return GetLocalSection()->GetProfiler();
    }

    Section* SectionMT::GetLocalSection()
    {
        auto id = GetCurrentThreadId();

        internal::spinlock_sentry sentry(m_lock);

        auto s = m_sectionsPerThreadId.find(id);

        if (s == m_sectionsPerThreadId.end())
        {
            Section* newSection = new Section(m_label);
            m_sectionsPerThreadId[id] = newSection;
            return newSection;
        }
        
        return s->second;
    }

    ProfilerNode& SectionMT::GetNode()
    {
        return GetLocalSection()->GetNode();
    }

    const Section* SectionMT::GetLocalSection() const
    {
        auto id = GetCurrentThreadId();
        internal::spinlock_sentry sentry(m_lock);
        auto s = m_sectionsPerThreadId.find(id);
        CHOBO_PROFILER_ASSERT(s != m_sectionsPerThreadId.end());
        return s->second;
    }

    size_t SectionMT::GetId() const
    {       
        
        return GetLocalSection()->GetId();
    }

    template class SplitSection<SectionMT>;
    template class Tagger<SectionMT>;
    template class Tagger<SplitSection<SectionMT>>;

#endif

    template class SplitSection<Section>;
    template class Tagger<Section>;
    template class Tagger<SplitSection<Section>>;
    
} } // namespace chobo.profiling
