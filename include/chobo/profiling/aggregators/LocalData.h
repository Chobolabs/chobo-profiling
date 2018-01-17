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

// aggregator which calculates local data
// time and memory data which is not profiled in any of the children

#include "../Config.h"

#include "ReportAggregator.h"

#include <cstdint>

namespace chobo { namespace profiling { namespace aggregators
{
    class CHOBO_PROFILING_API LocalData : public ReportAggregator
    {
    public:
        static const size_t id;

        LocalData(const ReportNode& node);

        virtual void Run() override;
        virtual void Reset() override;
        virtual void MergeWith(const ReportNode& node) override;

        uint64_t GetCurrentUnprofiledTime() const { return m_currentUnprofiledTime; }
        uint64_t GetTotalUnprofiledTime() const { return m_totalUnprofiledTime; }

        uint32_t GetLocalAllocations() const { return m_localAllocations; }
        size_t GetLocalAllocatedMemory() const { return m_localAllocatedMemory; }
        uint32_t GetLocalDeallocations() const { return m_localDeallocations; }

    private:
        uint64_t m_currentUnprofiledTime = 0;
        uint64_t m_totalUnprofiledTime = 0;

        uint32_t m_localAllocations = 0;
        size_t m_localAllocatedMemory = 0;
        uint32_t m_localDeallocations = 0;
    };

}}} // namespace chobo.profiling.aggregators
