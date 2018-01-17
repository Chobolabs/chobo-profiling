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

#include <chobo/profiling/aggregators/Sum.h>
#include <chobo/profiling/ReportNode.h>

namespace chobo { namespace profiling { namespace aggregators
{
    const size_t Sum::id = ReportAggregator::GetFreeId();

    Sum::Sum(const ReportNode& node)
        : ReportAggregator(node)
    {

    }

    void Sum::Run()
    {
        m_data.Append(m_node.GetProfilingData());
    }

    void Sum::Reset()
    {
        m_data.Clear();
    }

    void Sum::MergeWith(const ReportNode& node)
    {
        auto agg = node.GetAggregator<Sum>();
        assert(agg);

        m_data.Append(agg->m_data);
    }

}}} // namespace chobo.profiling.aggregators
