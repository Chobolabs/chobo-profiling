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

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

namespace chobo { namespace profiling
{
    class ReportNode;
    class ReportNodeTraverser;
    class Profiler;
    class ProfilerNode;
    class ReportAggregatorPolicy;

    class CHOBO_PROFILING_API Report
    {
    public:
        Report(std::shared_ptr<ReportAggregatorPolicy> aggregatorPolicy = nullptr);
        ~Report();

        Report(const Report&) = delete;
        Report& operator=(const Report&) = delete;

        // Update report
        // If the base node is nullptr, it starts from root
        // Always treat the base node as a root
        void UpdateFrom(Profiler& profiler, const ProfilerNode* baseNode = nullptr);

        // Update from multiple nodes - multiple roots
        void UpdateFrom(Profiler& profiler, const ProfilerNode* const* nodes, size_t numNodes);

        void UpdateFlatNodes();

        // DFS traversal of all nodes
        // return true if some node has cut the traversal short
        bool TraverseNodes(ReportNodeTraverser& traverser);

        // Traversal of the flat report
        // return true if some node has cut the traversal short
        bool TraverseFlatData(ReportNodeTraverser& traverser);

        std::vector<ReportNode*> GetFlatNodes() const;

        std::shared_ptr<const ReportAggregatorPolicy> GetAggregatorPolicy() const { return m_aggregatorPolicy; }

        ProfilingData CalculateAggregatedRootData() const;

        const std::vector<ReportNode*>& GetRoots() const { return m_roots; }

        // clones the report
        // The argument is used as the policy of the resulting clone
        // If no policy is provided the one contained in this report will be used
        // Optionally slow frames can be present in the clone
        Report* Clone(std::shared_ptr<ReportAggregatorPolicy> clonePolicy = nullptr, bool cloneSlowFrames = false) const;

        struct SlowFrameData
        {
            ~SlowFrameData();
            uint64_t frameTime = 0;
            std::shared_ptr<chobo::profiling::Report> report = nullptr;
        };
        
        // start collecting of N slowest frames on each update
        void StartCollectingSlowFrames(size_t numSlowFrames);
        void StopCollectingSlowFrames();
        bool IsCollectingSlowFrames() const { return m_isCollectingSlowFrames; }
        void ClearSlowFrames();
        void UpdateSlowFrames();
        const std::vector<SlowFrameData>& GetSlowFrames() const { return m_slowFrames; }

    private:
        void UpdateNode(ReportNode& reportNode, const ProfilerNode& profilerNode);
        void UpdateTree(ReportNode& reportNode, const ProfilerNode& profilerNode, const Profiler& profiler);

        // memory profiling data in a profiler is node-based (only allocating/deallocating nodes have it)
        // whereas that data in a report is aggregated (a node has the sum of all its children data)
        // the aggregation is done in this function
        void AggregateMemoryData(ReportNode& node);

        // Used by Clone. DFS-clones a node to one in another report
        static void CloneNode(ReportNode& target, const ReportNode& source);

        std::vector<ReportNode*> m_roots;

        ReportNode* GenerateNewNode(const std::string& sectionLabel);
        void DestroyNode(ReportNode* node);
        std::vector<std::unique_ptr<ReportNode>> m_nodes;

        std::vector<std::unique_ptr<ReportNode>> m_flatNodes;

        std::shared_ptr<ReportAggregatorPolicy> m_aggregatorPolicy;

        void RunAggregators();
        std::vector<ReportNode*> m_newNodes;

        bool m_isCollectingSlowFrames = false;
        std::shared_ptr<ReportAggregatorPolicy> m_slowFramesAggregatorPolicy;
        size_t m_maxNumSlowFrames = 10;
        std::vector<SlowFrameData> m_slowFrames;
    };

} } // namespace chobo.profiling
