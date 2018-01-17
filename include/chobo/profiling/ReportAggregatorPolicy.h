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

#include <vector>
#include <memory>

namespace chobo { namespace profiling
{
    class ReportAggregator;
    class ReportNode;

    class CHOBO_PROFILING_API ReportAggregatorPolicy
    {
    public:
        virtual ~ReportAggregatorPolicy() {}

        const std::vector<size_t>& GetAggregatorIds() const { return m_aggregatorIds; }

        virtual std::vector<std::shared_ptr<ReportAggregator>> GetAggregatorsForNewNode(const ReportNode&) const
        {
            return std::vector<std::shared_ptr<ReportAggregator>>();
        }

    protected:
        std::vector<size_t> m_aggregatorIds;
    };

} } // namespace chobo.profiling
