//
// chobo-profiling
// Copyright (c) 2015-2018 Chobolabs Inc. 
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#include "Internal.h"

#include <chobo/profiling/SimpleReportTraverser.h>
#include <chobo/profiling/ProfilerNode.h>
#include <chobo/profiling/Profiler.h>
#include <chobo/profiling/Report.h>
#include <chobo/profiling/ReportNode.h>

namespace chobo { namespace profiling
{
    SimpleReportTraverser::SimpleReportTraverser(std::ostream& out)
        : m_out(out)
    {

    }

    void SimpleReportTraverser::TraverseProfiler(Profiler& profiler)
    {
        m_indent.clear();
        m_currentProfiler = &profiler;
        profiler.TraverseNodes(*this);
    }

    void SimpleReportTraverser::TraverseReport(Report& report)
    {
        m_indent.clear();
        m_currentProfiler = nullptr;
        report.TraverseNodes(*this);
    }

    void SimpleReportTraverser::Down()
    {
        for (int i = 0; i < indent_size; ++i)
            m_indent.push_back(' ');
    }

    void SimpleReportTraverser::Up()
    {
        for (int i = 0; i < indent_size; ++i)
            m_indent.pop_back();
    }

    double SimpleReportTraverser::n2m(uint64_t nano)
    {
        return double(nano / 1000) / 1000;
    }

    bool SimpleReportTraverser::Traverse(ProfilerNode& node)
    {
        const char* label;
        if (m_currentProfiler)
        {
            label = m_currentProfiler->GetSectionLabel(node.GetSectionId()).c_str();
        }
        else
        {
            label = "node";
        }

        ReportData(label, node.GetProfilingData());

        return true;
    }

    bool SimpleReportTraverser::Traverse(ReportNode& node)
    {
        ReportData(node.GetSectionLabel().c_str(), node.GetProfilingData());

        return true;
    }

    void SimpleReportTraverser::ReportData(const char* label, const ProfilingData& data)
    {
        m_out << m_indent
            << label << ": "
            << n2m(data.time) << " ms, "
            << data.timesEntered << " calls";

        if (data.timesEntered != 0)
        {
            m_out << " (" << n2m(data.GetMeanTime()) << " mean time)";
        }

        m_out << ". a:" << data.allocations << " (" << data.allocatedMemory << ") d: " << data.deallocations;

        m_out << std::endl;
    }

} } // namespace chobo.profiling
