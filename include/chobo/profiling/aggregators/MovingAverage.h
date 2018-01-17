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

#include <deque>

namespace chobo { namespace profiling { namespace aggregators
{
    class CHOBO_PROFILING_API MovingAverage : public ReportAggregator
    {
    public:
        static const size_t id;

        MovingAverage(const ReportNode& node, size_t sampleCount, float outstandingThreshold = 1.f);

        virtual void Run() override;
        virtual void Reset() override;
        virtual void MergeWith(const ReportNode& node) override;

        const ProfilingData& GetAverage() const { return m_average; }
        unsigned GetMaxAllocs() const { return m_maxAllocs; }
        uint64_t GetMaxTime() const { return m_maxTime; }

        bool IsOutstanding() const { return m_isOutstanding; }

    private:
        const size_t m_maxSampleCount;
        
        ProfilingData m_sum;
        ProfilingData m_average;
        unsigned m_maxAllocs = 0;
        uint64_t m_maxTime = 0;
        bool m_isOutstanding = false;
        const float m_outstandingThreshold;

        std::deque<ProfilingData> m_sample;
    };

}}} // namespace chobo.profiling.aggregators
