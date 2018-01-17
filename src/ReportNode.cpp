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

#include <chobo/profiling/ReportNode.h>
#include <chobo/profiling/ReportNodeTraverser.h>
#include <chobo/profiling/Report.h>
#include <chobo/profiling/ReportAggregatorPolicy.h>

#include <algorithm>

namespace chobo { namespace profiling
{
    ReportNode::ReportNode(Report& report, size_t id, const std::string& sectionLabel)
        : m_report(report)
        , m_id(id)
        , m_sectionLabel(sectionLabel)
    {
    }

    ReportNode::~ReportNode()
    {
    }

    bool ReportNode::Traverse(ReportNodeTraverser& traverser)
    {
        if (!traverser.Traverse(*this))
        {
            return false;
        }

        for (auto& child : m_children)
        {
            traverser.Down();
            if (!child->Traverse(traverser))
            {
                return false;
            }

            traverser.Up();
        }

        return true;
    }

    void ReportNode::RunAggregators()
    {
        for (auto& aggregator : m_aggregators)
        {
            aggregator->Run();
        }
    }

    void ReportNode::ResetAggregators()
    {
        for (auto& aggregator : m_aggregators)
        {
            aggregator->Reset();
        }
    }

    const ReportAggregator* ReportNode::GetAggregator(size_t id) const
    {
        const auto policy = m_report.GetAggregatorPolicy();
        const auto& ids = policy->GetAggregatorIds();

        if (id >= ids.size() || ids[id] == size_t(-1))
            return nullptr;

        return m_aggregators[ids[id]].get();
    }

    ReportAggregator* ReportNode::GetAggregator(size_t id)
    {
        const auto policy = m_report.GetAggregatorPolicy();
        const auto& ids = policy->GetAggregatorIds();

        if (id >= ids.size() || ids[id] == size_t(-1))
            return nullptr;

        return m_aggregators[ids[id]].get();
    }

    void ReportNode::MergeWith(const ReportNode& other)
    {
        m_profilingData.Append(other.m_profilingData);

        for (auto& aggregator : m_aggregators)
        {
            aggregator->MergeWith(other);
        }
    }

    void ReportNode::Clear()
    {
        SimpleTraverse([](ReportNode& node)
        {
            node.m_profilingData.Clear();
            node.m_isUsed = false;
        });
    }
} } // namespace chobo.profiling
