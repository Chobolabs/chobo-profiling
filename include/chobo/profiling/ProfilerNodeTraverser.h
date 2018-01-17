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

namespace chobo { namespace profiling
{
    class ProfilerNode;

    // you can inherit from this class to traverse all nodes in a profiler
    class CHOBO_PROFILING_API ProfilerNodeTraverser
    {
    public:
        // called for each node
        // if it returns false, it breaks the traversal
        virtual bool Traverse(ProfilerNode& node) = 0;

        // called when moving the traversal from a parent to a child
        virtual void Down() {};

        // called when moving the traversal up a level
        virtual void Up() {};

        virtual ~ProfilerNodeTraverser() {};
    };
} } // namespace chobo.profiling
