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

#include <string>
#include <unordered_set>
#include <vector>

namespace chobo {
namespace profiling {

class ProfilerNode;

class CHOBO_PROFILING_API Tag
{
public:
    Tag(const char* name);

    const std::string& GetName() const { return m_name; }

    void AddNode(const ProfilerNode* node);
    void RemoveNode(const ProfilerNode* node);

    void ClearNodes();

    const std::vector<const ProfilerNode*>& GetNodes() const { return m_nodes; }

private:
    const std::string m_name;
    std::vector<const ProfilerNode*> m_nodes;
};

} } // namespace chobo.profiling
