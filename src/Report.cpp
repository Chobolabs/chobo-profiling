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

#include <chobo/profiling/Report.h>
#include <chobo/profiling/ReportNode.h>
#include <chobo/profiling/Profiler.h>
#include <chobo/profiling/ProfilerNode.h>
#include <chobo/profiling/ReportAggregatorPolicy.h>
#include <chobo/profiling/ReportNodeTraverser.h>

#include <algorithm>
#include <unordered_set>

using namespace std;

namespace chobo { namespace profiling
{
    Report::Report(std::shared_ptr<ReportAggregatorPolicy> aggregatorPolicy /*= nullptr*/)
    {
        if (!aggregatorPolicy)
        {
            aggregatorPolicy.reset(new ReportAggregatorPolicy);
        }

        m_aggregatorPolicy = aggregatorPolicy;
    }

    Report::~Report()
    {
    }

    void Report::UpdateFrom(Profiler& profiler, const ProfilerNode* baseNode /*= nullptr*/)
    {
        if (baseNode == nullptr)
        {
            baseNode = &profiler.GetRootNode();
        }

        // if the node is root and the profiler has a name use the node name as a profiler name
        const std::string& nodeName = 
            (baseNode == &profiler.GetRootNode() && !profiler.GetName().empty()) ?
                profiler.GetName()
            :
                profiler.GetSectionLabel(baseNode->GetSectionId());

        assert(m_newNodes.empty());

        for (auto root : m_roots)
        {
            if (root->GetSectionLabel() == nodeName)
            {
                // we have such a root, so reuse it
                UpdateTree(*root, *baseNode, profiler);
                AggregateMemoryData(*root);
                RunAggregators();
                return;
            }
        }

        // no such root
        // create a new one
        auto root = GenerateNewNode(nodeName);
        m_roots.push_back(root);
        UpdateTree(*root, *baseNode, profiler);
        AggregateMemoryData(*root);
        RunAggregators();
    }

    void Report::UpdateFrom(Profiler& profiler, const ProfilerNode* const* nodes, size_t numNodes)
    {
        assert(m_roots.size() <= numNodes);
        size_t i = 0;
        for (; i<m_roots.size(); ++i)
        {
            assert(nodes[i]);
            auto root = m_roots[i];
            UpdateTree(*root, *nodes[i], profiler);
            AggregateMemoryData(*root);
        }

        // create new roots for the rest of the nodes (if any)
        for (; i < numNodes; ++i)
        {
            auto node = nodes[i];
            assert(node);
            // if the node is root and the profiler has a name use the node name as a profiler name
            const std::string& nodeName =
                (node == &profiler.GetRootNode() && !profiler.GetName().empty()) ?
                profiler.GetName()
                :
                profiler.GetSectionLabel(node->GetSectionId());

            auto root = GenerateNewNode(nodeName);
            m_roots.push_back(root);

            UpdateTree(*root, *node, profiler);
            AggregateMemoryData(*root);
        }

        RunAggregators();
    }

    void Report::AggregateMemoryData(ReportNode& node)
    {
        for (auto c : node.m_children)
        {
            AggregateMemoryData(*c);
            node.m_profilingData.allocations += c->m_profilingData.allocations;
            node.m_profilingData.allocatedMemory += c->m_profilingData.allocatedMemory;
            node.m_profilingData.deallocations += c->m_profilingData.deallocations;            
        }
    }

    void Report::UpdateNode(ReportNode& reportNode, const ProfilerNode& profilerNode)
    {
        reportNode.m_profilingData = profilerNode.GetProfilingData();
        reportNode.m_isUsed = true;
    }

    void Report::UpdateTree(ReportNode& reportNode, const ProfilerNode& profilerNode, const Profiler& profiler)
    {
        if (profilerNode.GetProfilingData().timesEntered == 0)
        {
            // we can safely disregard the recursion for unused nodes
            if (reportNode.m_isUsed)
            {
                reportNode.Clear();
            }
            return;
        }

        UpdateNode(reportNode, profilerNode);

        const auto& profilerNodeChildren = profilerNode.GetChildren();

        if (profilerNodeChildren.empty())
        {
            return;
        }

        assert(reportNode.m_children.size() <= profilerNodeChildren.size());

        for (size_t i = 0; i < reportNode.m_children.size(); ++i)
        {
            //assert(reportNode.m_children[i]->m_sectionLabel == profiler.GetSectionLabel(profilerNodeChildren[i]->GetSectionId()));
            UpdateTree(*reportNode.m_children[i], *profilerNodeChildren[i], profiler);
        }

        for (size_t i = reportNode.m_children.size(); i < profilerNodeChildren.size(); ++i)
        {
            auto profilerChild = profilerNodeChildren[i];
            const auto& label = profiler.GetSectionLabel(profilerChild->GetSectionId());
            auto child = GenerateNewNode(label);
            child->m_parent = &reportNode;
            reportNode.m_children.push_back(child);
            UpdateTree(*child, *profilerChild, profiler);
        }
    }

    void Report::UpdateFlatNodes()
    {
        // since we're beginning a new flat merge, we need to clear the profiling data and reset the aggregators
        // otherwise we'll aggregate the same data multiple times
        for (auto& flatNode : m_flatNodes)
        {
            flatNode->m_isUsed = false;
            flatNode->m_profilingData.Clear();
            flatNode->ResetAggregators();
        }

        // update nodes from the tree
        for (auto& node : m_nodes)
        {
            ReportNode* flatNode;
            if (node->m_flatNode)
            {
                flatNode = node->m_flatNode;
                assert(flatNode->GetSectionLabel() == node->GetSectionLabel());
            }
            else
            {
                // new node
                // check flat nodes for it
                for (auto& fnode : m_flatNodes)
                {
                    if (fnode->GetSectionLabel() == node->GetSectionLabel())
                    {
                        flatNode = fnode.get();
                        break;
                    }
                }

                // not found, so create a new flat node
                flatNode = new ReportNode(*this, 0, node->GetSectionLabel());
                flatNode->m_aggregators = m_aggregatorPolicy->GetAggregatorsForNewNode(*flatNode);

                m_flatNodes.emplace_back(flatNode);
                node->m_flatNode = flatNode;
            }

            flatNode->MergeWith(*node);
            flatNode->m_isUsed = true;
        }

        // erase all flat nodes that weren't found in the tree
        m_flatNodes.erase(
            remove_if(m_flatNodes.begin(), m_flatNodes.end(), [](const unique_ptr<ReportNode>& node)
            {
                return !node->m_isUsed;
            }),
            m_flatNodes.end());
    }

    bool Report::TraverseNodes(ReportNodeTraverser& traverser)
    {
        bool ret = true;
        for (auto root : m_roots)
        {
            ret = ret && root->Traverse(traverser);
        }

        return ret;
    }

    bool Report::TraverseFlatData(ReportNodeTraverser& traverser)
    {
        for (auto& node : m_flatNodes)
        {
            if (!traverser.Traverse(*node))
            {
                return false;
            }
        }

        return true;
    }

    ReportNode* Report::GenerateNewNode(const std::string& sectionLabel)
    {
        ReportNode* ret = nullptr;

        for (size_t i = 0; i < m_nodes.size(); ++i)
        {
            auto& node = m_nodes[i];
            if (!node)
            {
                node.reset(new ReportNode(*this, i, sectionLabel));
                ret = node.get();
            }
        }

        if (!ret)
        {
            // no free spot was found
            m_nodes.emplace_back(new ReportNode(*this, m_nodes.size(), sectionLabel));
            ret = m_nodes.back().get();
        }

        m_newNodes.push_back(ret);
        return ret;
    }

    void Report::DestroyNode(ReportNode* node)
    {
        for (auto child : node->m_children)
        {
            DestroyNode(child);
        }

        m_nodes[node->GetId()].reset(nullptr);
    }

    void Report::RunAggregators()
    {
        for (auto newNode : m_newNodes)
        {
            newNode->m_aggregators = m_aggregatorPolicy->GetAggregatorsForNewNode(*newNode);            
        }

        for (auto& node : m_nodes)
        {
            node->RunAggregators();
        }

        m_newNodes.clear();
    }

    vector<ReportNode*> Report::GetFlatNodes() const
    {
        vector<ReportNode*> ret;
        ret.reserve(m_flatNodes.size());

        for (auto& node : m_flatNodes)
        {
            ret.push_back(node.get());
        }

        return ret;
    }

    ProfilingData Report::CalculateAggregatedRootData() const
    {
        ProfilingData data;

        for (auto root : m_roots)
        {
            data.Append(root->GetProfilingData());
        }

        return data;
    }

    void Report::CloneNode(ReportNode& target, const ReportNode& source)
    {
        target.MergeWith(source);
        target.m_profilingData.extraData = source.m_profilingData.extraData;

        auto& clonedReport = target.m_report;

        target.m_children.reserve(source.m_children.size());

        for (auto& child : source.m_children)
        {
            auto clonedNode = new ReportNode(clonedReport, clonedReport.m_nodes.size(), child->GetSectionLabel());
            clonedNode->m_aggregators = clonedReport.m_aggregatorPolicy->GetAggregatorsForNewNode(*clonedNode);

            clonedReport.m_nodes.emplace_back(clonedNode);
            target.m_children.emplace_back(clonedNode);

            CloneNode(*clonedNode, *child);
        }
    }

    Report* Report::Clone(std::shared_ptr<ReportAggregatorPolicy> clonePolicy, bool cloneSlowFrames) const
    {
        if (!clonePolicy)
        {
            clonePolicy = m_aggregatorPolicy;
        }

        auto clonedReport = new Report(clonePolicy);
        clonedReport->m_roots.reserve(m_roots.size());
        clonedReport->m_nodes.reserve(m_nodes.size());

        for (auto root : m_roots)
        {
            auto& cloneNodes = clonedReport->m_nodes;

            auto clonedNode = new ReportNode(*clonedReport, cloneNodes.size(), root->GetSectionLabel());
            clonedNode->m_aggregators = clonePolicy->GetAggregatorsForNewNode(*clonedNode);

            cloneNodes.emplace_back(clonedNode);
            clonedReport->m_roots.emplace_back(clonedNode);

            CloneNode(*clonedNode, *root);
        }

        // don't generate flat nodes of clone, if we don't have any
        if (!m_flatNodes.empty())
        {
            clonedReport->m_flatNodes.reserve(m_flatNodes.size());

            for (auto& flatNode : m_flatNodes)
            {
                auto clonedNode = new ReportNode(*clonedReport, 0, flatNode->GetSectionLabel());
                clonedNode->m_aggregators = clonePolicy->GetAggregatorsForNewNode(*clonedNode);
                clonedNode->MergeWith(*flatNode);

                clonedReport->m_flatNodes.emplace_back(clonedNode);
            }
        }

        if (cloneSlowFrames)
        {
            clonedReport->m_slowFrames = m_slowFrames;
        }

        return clonedReport;
    }

    void Report::StartCollectingSlowFrames(size_t numSlowFrames)
    {
        m_maxNumSlowFrames = numSlowFrames;
        if (m_slowFrames.size() > m_maxNumSlowFrames)
        {
            m_slowFrames.resize(m_maxNumSlowFrames);
        }
        m_slowFrames.reserve(m_maxNumSlowFrames);
        m_isCollectingSlowFrames = true;
    }

    void Report::StopCollectingSlowFrames()
    {
        m_isCollectingSlowFrames = false;
    }

    void Report::ClearSlowFrames()
    {
        m_slowFrames.clear();
    }

    void Report::UpdateSlowFrames()
    {
        if (!m_isCollectingSlowFrames) return;

        if (!m_slowFramesAggregatorPolicy)
        {
            m_slowFramesAggregatorPolicy = make_shared<chobo::profiling::ReportAggregatorPolicy>();
        }

        auto time = CalculateAggregatedRootData().time;
        //uint64_t time = 0;
        //for (auto root : m_roots)
        //{
        //    for (auto sub : root->m_children)
        //    {
        //        time += sub->GetProfilingData().time;
        //    }
        //}

        size_t i = 0;
        for (; i<m_slowFrames.size(); ++i)
        {
            auto& f = m_slowFrames[i];
            if (f.frameTime < time)
                break;
        }

        if (m_slowFrames.size() == m_maxNumSlowFrames)
        {
            if (i == m_slowFrames.size())
            {
                // current frame is faster than all frames we've collected and there is no room for more
                return;
            }
            else
            {
                // make room for the current frame
                m_slowFrames.pop_back();
            }
        }

        SlowFrameData frame;
        frame.frameTime = time;
        frame.report.reset(this->Clone(m_slowFramesAggregatorPolicy));
        m_slowFrames.insert(m_slowFrames.begin() + i, frame);
    }

    // not actually needed but it must be compiled in the shared library
    Report::SlowFrameData::~SlowFrameData() {}

} } // namespace chobo.profiling
