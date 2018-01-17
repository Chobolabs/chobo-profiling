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
#include <chobo/profiling/aggregators/ReportAggregator.h>

namespace chobo { namespace profiling
{
    ReportAggregator::ReportAggregator(const ReportNode& node)
        : m_node(node)
    {

    }

    ReportAggregator::~ReportAggregator()
    {

    }

    size_t ReportAggregator::GetFreeId()
    {
        static size_t id = 0;
        return id++;
    }

} } // namespace chobo.profiling
