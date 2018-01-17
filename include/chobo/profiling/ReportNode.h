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

#include "ProfilingData.h"
#include "aggregators/ReportAggregator.h"

#include <vector>
#include <memory>
#include <string>

namespace chobo { namespace profiling
{
    class Report;
    class ReportNodeTraverser;
    class ReportAggregator;

    class CHOBO_PROFILING_API ReportNode
    {
    public:
        ReportNode(Report& report, size_t id, const std::string& sectionLabel);
        ~ReportNode();

        ReportNode(const ReportNode&) = delete;
        ReportNode& operator=(const ReportNode&) = delete;

        const ProfilingData& GetProfilingData() const { return m_profilingData; }

        size_t GetId() const { return m_id; }
        const std::string& GetSectionLabel() const { return m_sectionLabel; }

        // user data for node (not one collected from the report)
        // use it to associate some data of your own with object
        intptr_t GetReportNodeUserData() const { return m_userData; }
        void SetReportNodeUserData(intptr_t data) { m_userData = data; }

        // Traverse the nodes with a functor, instead of a traverse.
        // The functor needs to be equivalent to a function foo(ReportNode&)
        // Doesn't check for return value
        // Doesn't notify for tree level
        template <typename Func>
        void SimpleTraverse(Func traverser)
        {
            traverser(*this);

            for (auto& child : m_children)
            {
                child->SimpleTraverse(traverser);
            }
        }

        // DFS traversal of this and children
        // return true if some node has cut the traversal short
        bool Traverse(ReportNodeTraverser& traverser);

        void RunAggregators();
        void ResetAggregators();

        const std::vector<ReportNode*>& GetChildren() const { return m_children; }

        template <typename Aggregator>
        const Aggregator* GetAggregator() const
        {
            auto ret = GetAggregator(Aggregator::id);
            return static_cast<const Aggregator*>(ret);
        }

        const ReportAggregator* GetAggregator(size_t id) const;

        template <typename Aggregator>
        Aggregator* GetAggregator()
        {
            auto ret = GetAggregator(Aggregator::id);
            return static_cast<Aggregator*>(ret);
        }

        ReportAggregator* GetAggregator(size_t id);

        void MergeWith(const ReportNode& other);

        // Clears profiling data of this node and its children
        void Clear(); 

    private:
        friend class Report;

        Report& m_report;
        const size_t m_id;
        const std::string m_sectionLabel;

        std::string m_description;

        ProfilingData m_profilingData;

        std::vector<std::shared_ptr<ReportAggregator>> m_aggregators;

        intptr_t m_userData = 0; // user data for this node

        ///////////////////////////////////////////////////////////////////////
        // Tree data
        ReportNode* m_parent = nullptr;
        std::vector<ReportNode*> m_children;

        ///////////////////////////////////////////////////////////////////////
        // Helper and optimization data
        ReportNode* m_flatNode = nullptr; // corresponding node in the flat report
        bool m_isUsed = false;  // for flat nodes shows whether the node exists in the tree when building a flat report
                                // for tree nodes shows that the node and its children are clean 
                                // (used to prevent updates for unused nodes when generating reports)
    };
} } // namespace chobo.profiling
