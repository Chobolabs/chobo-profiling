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

#include <chobo/profiling/aggregators/MovingAverage.h>
#include <chobo/profiling/ReportNode.h>

#include <algorithm>

namespace chobo { namespace profiling { namespace aggregators
{
    const size_t MovingAverage::id = ReportAggregator::GetFreeId();

    MovingAverage::MovingAverage(const ReportNode& node, size_t sampleCount, float outstandingThreshold)
        : ReportAggregator(node)
        , m_maxSampleCount(sampleCount)
        , m_outstandingThreshold(outstandingThreshold)
    {
    }

    void MovingAverage::Run()
    {
        if (m_sample.size() >= m_maxSampleCount)
        {
            const auto& oldest = m_sample.front();
            const auto& newest = m_sample.back();
            
            if (oldest.time == m_maxTime)
            {
                m_maxTime = newest.time;
            }

            if (oldest.allocations == m_maxAllocs)
            {
                m_maxAllocs = newest.allocations;
            }

            m_sum.time -= oldest.time;
            m_sum.timesEntered -= oldest.timesEntered;
            m_sum.allocations -= oldest.allocations;
            m_sum.allocatedMemory -= oldest.allocatedMemory;
            m_sum.deallocations -= oldest.deallocations;

            m_sample.pop_front();
        }

        m_sample.push_back(m_node.GetProfilingData());

        auto& newest = m_sample.back();
        if (newest.time > m_maxTime)
        {
            m_maxTime = newest.time;
        }

        if (newest.allocations > m_maxAllocs)
        {
            m_maxAllocs = newest.allocations;
        }

        m_sum.Append(newest);

        m_average.time = m_sum.time / m_sample.size();
        m_average.timesEntered = m_sum.timesEntered / unsigned(m_sample.size());
        m_average.allocations = m_sum.allocations / unsigned(m_sample.size());
        m_average.allocatedMemory = m_sum.allocatedMemory / m_sample.size();
        m_average.deallocations = m_sum.deallocations / unsigned(m_sample.size());

        m_isOutstanding = false;
        if (m_node.GetProfilingData().time > (1 + m_outstandingThreshold) * m_average.time)
        {
            m_isOutstanding = true;
        }
    }

    void MovingAverage::Reset()
    {
        m_isOutstanding = false;
        m_average.Clear();
        m_maxAllocs = 0;
        m_maxTime = 0;
        m_sample.clear();
    }

    void MovingAverage::MergeWith(const ReportNode& node)
    {
        auto agg = node.GetAggregator<MovingAverage>();
        assert(agg);

        m_average.Append(agg->GetAverage());
        m_maxAllocs += agg->GetMaxAllocs();
        m_maxTime += agg->GetMaxTime();
        m_isOutstanding = m_isOutstanding || agg->IsOutstanding();
    }

}}} // namespace chobo.profiling.aggregators
