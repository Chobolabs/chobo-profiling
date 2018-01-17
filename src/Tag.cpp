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

#include <chobo/profiling/Tag.h>

#include <algorithm>

namespace chobo {
namespace profiling {

Tag::Tag(const char* name)
    : m_name(name)
{}

void Tag::AddNode(const ProfilerNode* node)
{
    m_nodes.push_back(node);
}

void Tag::RemoveNode(const ProfilerNode* node)
{
    auto f = std::find(m_nodes.begin(), m_nodes.end(), node);
    CHOBO_PROFILER_ASSERT(f != m_nodes.end());
    if (f != m_nodes.end())
    {
        m_nodes.erase(f);
    }
}

void Tag::ClearNodes()
{
    m_nodes.clear();
}

}
} // namespace chobo.profiling