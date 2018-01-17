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

#include "ProfilerNode.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace chobo { namespace profiling
{
    class ProfilerNodeTraverser;
    class Report;
    class ReportAggregatorPolicy;
    class Tag;

    class CHOBO_PROFILING_API Profiler
    {
    public:
        // The profiler name is for the user's convenience only
        // You might want to set it to something that will make it clearer which profiler this is
        void SetName(const std::string& name) { m_name = name; }
        const std::string& GetName() const { return m_name; }

        // enter a profiling section by id
        // create a profiling node, if needed
        void Enter(const size_t sectionId);

        // leave a profiling section
        // calling leave more times than enter will cause an assertion to fail (or a crash)
        void Leave();

        // leave a named profiling section
        // internally it calls leave 
        // but when debugging perorms a sanity check whether the section being left is correct
        void Leave(const char* label);

        // returns an id for the new section
        // if a section of 
        size_t AddSection(const char* label);

        // gets the root node
        ProfilerNode& GetRootNode();
        const ProfilerNode& GetRootNode() const;

        // gets the current node in the profiling session
        ProfilerNode& GetCurrentNode() { return *m_curNode; }
        const ProfilerNode& GetCurrentNode() const { return *m_curNode; }

        // DFS traversal of all nodes
        // return true if some node has cut the traversal short
        bool TraverseNodes(ProfilerNodeTraverser& traverser);

        // Traverse the nodes with a functor, instead of a traverse.
        // The functor needs to be equivalent to a function foo(ProfilerNode&)
        // Doesn't check for return value
        // Doesn't notify for tree level
        template <typename Func>
        void SimpleTraverseNodes(Func traverser)
        {
            GetRootNode().SimpleTraverse(traverser);
        }

        // resets the profile data (including the node structure)
        // it's usually a good idea to call this function at the start of your program
        // or after you want to start a new profiling session
        void Reset();

        // Clear profiling data in all nodes
        // Unlike reset it keeps the current node structure
        void Clear();

        const std::string& GetSectionLabel(size_t sectionId) const { return m_sectionDatas[sectionId].label; }

        // Calculate a report from the current node or the root node
        // Will create the report on the first call and update it on each subsequent one
        Report& CalculateReport(std::shared_ptr<ReportAggregatorPolicy> aggregatorPolicy = nullptr);
        Report& CalculateRootReport(std::shared_ptr<ReportAggregatorPolicy> aggregatorPolicy = nullptr);

        // Get a calculated report from the current node or the root node
        // If the report doesn't exist, the function won't create it and will return nullptr instead
        Report* GetReport();
        Report* GetRootReport();

        // Shows whether the profiler is paused (in which case no profiling info should be provided)
        bool IsPaused() { return m_isPaused; }

        // Profile allocations and deallocations
        void ProfileAlloc(size_t size);
        void ProfileDealloc();

        // Tags
        // creates the tag if it doesn't exist
        Tag& GetTag(size_t sectionId, const char* name);
        // returns nullptr if tag doesn't exist
        const Tag* GetTag(const char* name) const;
        const std::vector<std::unique_ptr<Tag>>& GetTags() const { return m_tags; }

    private:
        friend class ProfilingManager;
        Profiler();
        ~Profiler();
        Profiler(const Profiler&) = delete;
        Profiler& operator=(const Profiler&) = delete;

        std::string m_name;

        struct SectionData
        {
            SectionData() = default;
            SectionData(const char* l)
                : label(l)
                , tag(nullptr)
            {}
            std::string label;
            Tag* tag = nullptr;
        };

        // labels of the sections associated with this profiler
        std::vector<SectionData> m_sectionDatas;

        // a pool of profiler nodes
        // it's not a good idea to allocate new memory during the profiling
        // that's why a pool of nodes is being used
        std::vector<std::vector<ProfilerNode>> m_nodePool;
        size_t m_curNodePoolPage = 0;

        // current node (for entering and leaving)
        ProfilerNode* m_curNode;

        ProfilerNode* GenerateNewNode(const size_t sectionId);

        // updates the current node to point to the appropriate child
        // creates the child if needed
        void UpdateCurNodeForEnter(const size_t sectionId);

        // Calculate a report from a specific node
        Report& CalculateReport(ProfilerNode* node, std::shared_ptr<ReportAggregatorPolicy> aggregatorPolicy);
        
        // Get the report from a specific node if it (the report) exists
        Report* GetReport(const ProfilerNode* node);

        std::unordered_map<const ProfilerNode*, std::unique_ptr<Report>> m_reportsPerNode;

        void ClearReports();

        friend class ProfilerPauseSentry;
        void Pause();
        void Resume();

        bool m_isPaused = false;

        // tags
        std::vector<std::unique_ptr<Tag>> m_tags;
    };

} } // namespace chobo.profiling
