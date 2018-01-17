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
#include <chobo/profiling/Profiler.h>
#include <chobo/profiling/Report.h>
#include <chobo/profiling/Tag.h>

#include <algorithm>

namespace chobo { namespace profiling
{
    Profiler::Profiler()
        : m_nodePool(1)
    {
        m_sectionDatas.reserve(Profiler_NodeStaticChildren);
        m_sectionDatas.emplace_back("root node");
        m_nodePool[0].reserve(Profiler_NodePoolPageSize);
        m_curNode = GenerateNewNode(0); // root node
        m_curNode->Enter();
    }

    Profiler::~Profiler()
    {
        assert(m_curNode->m_parent == nullptr);
        ClearReports();
    }

    void Profiler::Enter(const size_t sectionId)
    {
        UpdateCurNodeForEnter(sectionId);
        m_curNode->Enter();
    }

    void Profiler::Leave()
    {
        m_curNode->Leave();

        // leaving the root node
        // a programming error
        CHOBO_PROFILER_ASSERT(m_curNode->m_parent);
        m_curNode = m_curNode->m_parent;
    }

    void Profiler::Leave(const char* label)
    {
        CHOBO_PROFILER_ASSERT(GetSectionLabel(m_curNode->GetSectionId()) == label);
        Leave();
    }

    size_t Profiler::AddSection(const char* label)
    {
        for (size_t i = 0; i < m_sectionDatas.size(); ++i)
        {
            if (m_sectionDatas[i].label == label) return i;
        }

        m_sectionDatas.emplace_back(label);
        return m_sectionDatas.size() - 1;
    }

    ProfilerNode* Profiler::GenerateNewNode(const size_t sectionId)
    {
        if (m_nodePool[m_curNodePoolPage].size() == Profiler_NodePoolPageSize)
        {
            // reached end of page, increment cur page index
            ++m_curNodePoolPage;

            // we don't remove pages, once we've created them
            // could be that there are free pages after this one
            // if not, create a new one
            if (m_curNodePoolPage == m_nodePool.size())
            {
                m_nodePool.resize(m_nodePool.size() + 1);
                m_nodePool.back().reserve(Profiler_NodePoolPageSize);
            }
        }

        auto& curPage = m_nodePool[m_curNodePoolPage];
        curPage.emplace_back(sectionId);

        auto& sectionData = m_sectionDatas[sectionId];
        if (sectionData.tag)
        {
            curPage.back().SetTag(sectionData.tag);
        }

        return &curPage.back();
    }

    ProfilerNode& Profiler::GetRootNode()
    {
        return m_nodePool[0][0];
    }

    const ProfilerNode& Profiler::GetRootNode() const
    {
        return m_nodePool[0][0];
    }

    void Profiler::UpdateCurNodeForEnter(const size_t sectionId)
    {
        auto child = m_curNode->FindChild(sectionId);

        if (!child)
        {
            // no existing child with this section has been found
            child = GenerateNewNode(sectionId);
            m_curNode->AddChild(child);
        }

        m_curNode = child;
    }

    bool Profiler::TraverseNodes(ProfilerNodeTraverser& traverser)
    {
        return GetRootNode().Traverse(traverser);
    }

    void Profiler::Reset()
    {
        // now this is not STRICTLY an error
        // but the user just shouldn't reset the profiler
        // unless for every call of enter there has been a call of leave
        // getting here quite likely means a premature reset
        // not all nodes that have been entered have been left
        assert(m_curNode->m_parent == nullptr);

        for (auto& tag : m_tags)
        {
            tag->ClearNodes();
        }

        for (auto& page : m_nodePool)
        {
            page.clear();
        }

        m_curNodePoolPage = 0;

        m_curNode = GenerateNewNode(0); // root

        // also clear profiling reports since they're now invalid
        ClearReports();

        m_curNode->Enter();
    }

    void Profiler::Clear()
    {
        for (auto& page : m_nodePool)
        {
            for (auto& node : page)
            {
                node.m_profilingData.Clear();
            }
        }

        // persist the times entered structure if we're calling this from an existing stack
        auto node = m_curNode;

        while (node)
        {
            node->m_profilingData.timesEntered = 1;
            node = node->m_parent;
        }
    }

    Report& Profiler::CalculateReport(std::shared_ptr<ReportAggregatorPolicy> aggregatorPolicy /*= nullptr*/)
    {
        return CalculateReport(m_curNode, aggregatorPolicy);
    }

    Report& Profiler::CalculateRootReport(std::shared_ptr<ReportAggregatorPolicy> aggregatorPolicy /*= nullptr*/)
    {
        return CalculateReport(&GetRootNode(), aggregatorPolicy);
    }

    Report& Profiler::CalculateReport(ProfilerNode* node, std::shared_ptr<ReportAggregatorPolicy> aggregatorPolicy)
    {
        auto f = m_reportsPerNode.find(node);

        Report* report;

        if (f == m_reportsPerNode.end())
        {
            report = new Report(aggregatorPolicy);
            m_reportsPerNode[node].reset(report);
        }
        else
        {
            report = f->second.get();
        }

        report->UpdateFrom(*this, node);
        
        return *report;
    }

    void Profiler::ClearReports()
    {
        Pause();
        m_reportsPerNode.clear();
        Resume();
    }

    void Profiler::Pause()
    {
        m_isPaused = true;
        auto node = m_curNode;

        while (node)
        {
            node->Leave();
            node = node->m_parent;
        }
    }

    void Profiler::Resume()
    {
        m_curNode->TraverseStack([](ProfilerNode& node)
        {
            node.Enter();
            --node.m_profilingData.timesEntered;
        });
        m_isPaused = false;
    }

    Report* Profiler::GetReport()
    {
        return GetReport(m_curNode);
    }

    Report* Profiler::GetRootReport()
    {
        return GetReport(&GetRootNode());
    }

    Report* Profiler::GetReport(const ProfilerNode* node)
    {
        auto f = m_reportsPerNode.find(node);

        if (f == m_reportsPerNode.end())
        {
            return nullptr;
        }
        else
        {
            return f->second.get();
        }
    }

    void Profiler::ProfileAlloc(size_t size)
    {
        if (m_isPaused) return;
        m_curNode->ProfileAlloc(size);
    }

    void Profiler::ProfileDealloc()
    {
        if (m_isPaused) return;
        m_curNode->ProfileDealloc();
    }

    const Tag* Profiler::GetTag(const char* name) const
    {
        auto f = find_if(m_tags.begin(), m_tags.end(), [name](const std::unique_ptr<Tag>& t) -> bool {
            return t->GetName() == name;
        });

        return f == m_tags.end() ? nullptr : f->get();
    }

    Tag& Profiler::GetTag(size_t sectionId, const char* name)
    {
        CHOBO_PROFILER_ASSERT(sectionId < m_sectionDatas.size());
        auto& data = m_sectionDatas[sectionId];
        if (data.tag) return *data.tag;
        auto tag = new Tag(name);
        m_tags.emplace_back(tag);
        data.tag = tag;
        return *tag;
    }

} } // namespace chobo.profiling
