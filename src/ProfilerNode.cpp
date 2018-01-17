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
#include <chobo/profiling/ProfilerNode.h>
#include <chobo/profiling/ProfilerNodeTraverser.h>
#include <chobo/profiling/Tag.h>

namespace chobo { namespace profiling 
{
    ProfilerNode::ProfilerNode(size_t sectionId)
        : m_sectionId(sectionId)
    {

    }

    ProfilerNode::~ProfilerNode()
    {

    }

    void ProfilerNode::Enter()
    {
        ++m_profilingData.timesEntered;
        m_enterTime = high_res_clock::now();
    }

    void ProfilerNode::Leave()
    {
        auto duration = high_res_clock::now() - m_enterTime;
        m_profilingData.time += std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
    }

    void ProfilerNode::AddChild(ProfilerNode* child)
    {
        child->m_parent = this;
        child->m_profilingData.callStackLevel = m_profilingData.callStackLevel + 1;

        m_children.push_back(child);
    }

    ProfilerNode* ProfilerNode::FindChild(size_t sectionId)
    {
        for (auto child : m_children)
        {
            if (child->m_sectionId == sectionId)
            {
                return child;
            }
        }

        return nullptr;
    }

    bool ProfilerNode::Traverse(ProfilerNodeTraverser& traverser)
    {
        if (!traverser.Traverse(*this))
        {
            return false;
        }

        for (auto child : m_children)
        {
            traverser.Down();
            if (!child->Traverse(traverser))
            {
                return false;
            }

            traverser.Up();
        }

        return true;
    }

    void ProfilerNode::Clear()
    {
        SimpleTraverse([](ProfilerNode& node)
        {
            node.m_profilingData.Clear();
        });
    }

    void ProfilerNode::ProfileAlloc(size_t size)
    {
        ++m_profilingData.allocations;
        m_profilingData.allocatedMemory += size;
    }

    void ProfilerNode::ProfileDealloc()
    {
        ++m_profilingData.deallocations;
    }

    void ProfilerNode::SetTag(Tag* tag)
    { 
        if (m_tag)
        {
            m_tag->RemoveNode(this);
        }
        m_tag = tag;
        if (m_tag)
        {
            m_tag->AddNode(this);
        }
    }

} } // namespace chobo.profiling
