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

#include "internal/high_res_clock.h"

#include "chobo/small_vector.hpp"

namespace chobo { namespace profiling
{
    class ProfilerNodeTraverser;
    class Tag;

    class CHOBO_PROFILING_API ProfilerNode
    {
    public:
        ProfilerNode(const size_t sectionId);
        ~ProfilerNode();

        ProfilerNode(const ProfilerNode&) = delete;
        ProfilerNode& operator=(const ProfilerNode&) = delete;

        // this shouldn't be allowed too, but until we rewrite a custom node pool class
        // in Profiler we'll live with this function
        ProfilerNode(ProfilerNode&&) = default;

        size_t GetSectionId() const { return m_sectionId; }

        const ProfilingData& GetProfilingData() const { return m_profilingData; }

        // extra data
        // use it to associate some data of your own with object
        intptr_t GetExtraData() const { return m_profilingData.extraData; }
        void SetExtraData(intptr_t data) { m_profilingData.extraData = data; }

        // clear profiling data in this node and all children
        void Clear();

        // Use with caution:
        // If this is not the current node of the profiler or one of the current node's parents,
        // this information will be probably useless
        const high_res_clock::time_point& GetEnterTime() const { return m_enterTime; }

        // Traverse the nodes with a functor, instead of a traverse.
        // The functor needs to be equivalent to a function foo(ProfilerNode&)
        // Doesn't check for return value
        // Doesn't notify for tree level
        template <typename Func>
        void SimpleTraverse(Func traverser)
        {
            traverser(*this);

            for (auto child : m_children)
            {
                child->SimpleTraverse(traverser);
            }
        }

        // DFS traversal of this and children
        // return true if some node has cut the traversal short
        bool Traverse(ProfilerNodeTraverser& traverser);

        template <typename Func>
        void TraverseStack(Func traverser)
        {
            if (m_parent)
            {
                m_parent->TraverseStack(traverser);
            }

            traverser(*this);
        }

        
        const chobo::small_vector<ProfilerNode*, Profiler_NodeStaticChildren>& GetChildren() const { return m_children; }

        // Add and memory allocation data to the profiling data
        void ProfileAlloc(size_t size);
        void ProfileDealloc();

        // Tagging
        void SetTag(Tag* tag);
        const Tag* GetTag() const { return m_tag; }

    private:
        friend class Profiler;
        void Enter();
        void Leave();

        // Aadd a child to this node
        void AddChild(ProfilerNode* child);

        // Find a child of this node by section id
        // Performs a linear search
        // Returns nullptr if the child is not found
        ProfilerNode* FindChild(size_t sectionId);

        const size_t m_sectionId;
        
        ProfilingData m_profilingData;

        ///////////////////////////////////////////////////////////////////////
        // profiling session data
        high_res_clock::time_point m_enterTime;

        ///////////////////////////////////////////////////////////////////////
        // Tree data
        ProfilerNode* m_parent = nullptr;
        chobo::small_vector<ProfilerNode*, Profiler_NodeStaticChildren> m_children;

        ///////////////////////////////////////////////////////////////////////
        // Tag data
        Tag* m_tag = nullptr;
    };
} } // namespace chobo.profiling