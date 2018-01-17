//
// chobo-profiling
// Copyright (c) 2015-2018 Chobolabs Inc. 
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#include "../Internal.h"

#include <chobo/profiling/aggregators/LocalData.h>
#include <chobo/profiling/ReportNode.h>

namespace chobo { namespace profiling { namespace aggregators
{
    const size_t LocalData::id = ReportAggregator::GetFreeId();

    LocalData::LocalData(const ReportNode& node)
        : ReportAggregator(node)
    {

    }

    void LocalData::Run()
    {
        auto& children = m_node.GetChildren();

        uint64_t childrenTime = 0;
        uint32_t childrenAllocations = 0;
        size_t childrenAllocatedMemory = 0;
        uint32_t childrenDeallocations = 0;
        for (auto child : children)
        {
            auto& data = child->GetProfilingData();
            childrenTime += data.time;
            childrenAllocations += data.allocations;
            childrenAllocatedMemory += data.allocatedMemory;
            childrenDeallocations += data.deallocations;
        }

        auto& data = m_node.GetProfilingData();
        m_currentUnprofiledTime = data.time - childrenTime;
        m_totalUnprofiledTime += m_currentUnprofiledTime;

        m_localAllocations = data.allocations - childrenAllocations;
        m_localAllocatedMemory = data.allocatedMemory - childrenAllocatedMemory;
        m_localDeallocations = data.deallocations - childrenDeallocations;
    }

    void LocalData::Reset()
    {
        m_currentUnprofiledTime = 0;
        m_totalUnprofiledTime = 0;
    }

    void LocalData::MergeWith(const ReportNode& node)
    {
        auto agg = node.GetAggregator<LocalData>();
        assert(agg);

        m_currentUnprofiledTime += agg->m_currentUnprofiledTime;
        m_totalUnprofiledTime += agg->m_totalUnprofiledTime;

        m_localAllocations += agg->m_localAllocations;
        m_localAllocatedMemory += agg->m_localAllocatedMemory;
        m_localDeallocations += agg->m_localDeallocations;
    }

}}} // namespace chobo.profiling.aggregators
