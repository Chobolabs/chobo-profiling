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

#include "../Config.h"

#include "ReportAggregator.h"
#include "../ProfilingData.h"

namespace chobo { namespace profiling { namespace aggregators
{
    class CHOBO_PROFILING_API Sum : public ReportAggregator
    {
    public:
        static const size_t id;

        Sum(const ReportNode& node);

        virtual void Run() override;
        virtual void Reset() override;
        virtual void MergeWith(const ReportNode& node) override;

        const ProfilingData& GetData() const { return m_data; }

    private:
        ProfilingData m_data;
    };

}}} // namespace chobo.profiling.aggregators
