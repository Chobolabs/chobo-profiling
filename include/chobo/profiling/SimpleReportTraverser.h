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

#include "ProfilerNodeTraverser.h"
#include "ReportNodeTraverser.h"

#include <ostream>
#include <cstdint>

namespace chobo { namespace profiling
{
    class ProfilerNode;
    class Profiler;
    class Report;
    struct ProfilingData;

    class CHOBO_PROFILING_API SimpleReportTraverser : public ProfilerNodeTraverser, public ReportNodeTraverser
    {
    public:
        SimpleReportTraverser(std::ostream& out);

        void TraverseProfiler(Profiler& profiler);
        void TraverseReport(Report& report);

    protected:
        static const int indent_size = 2;

        static double n2m(uint64_t nano);

        void Down() override;
        void Up() override;

        bool Traverse(ProfilerNode& node) override;
        bool Traverse(ReportNode& node) override;

        void ReportData(const char* label, const ProfilingData& data);

        const Profiler* m_currentProfiler = nullptr;
        std::string m_indent;
        std::ostream& m_out;
    };

} } // namespace chobo.profiling
