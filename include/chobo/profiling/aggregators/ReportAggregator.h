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

#include <cstddef>

namespace chobo { namespace profiling
{
    class ReportNode;

    class CHOBO_PROFILING_API ReportAggregator
    {
    public:
        ReportAggregator(const ReportNode& node);
        virtual ~ReportAggregator();

        virtual void Run() = 0;
        virtual void Reset() = 0;
        virtual void MergeWith(const ReportNode& node) = 0;

    protected:
        // used to generate unique id's for all aggregators types
        // thus they can be easily identified by it
        static size_t GetFreeId();

        const ReportNode& m_node;

    private:
        friend class ReportNode;
    };

} } // namespace chobo.profiling
