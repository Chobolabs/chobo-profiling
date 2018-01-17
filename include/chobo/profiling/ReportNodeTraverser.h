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
    class ReportNode;

    // you can inherit from this class to traverse all nodes in a profiler report
    class CHOBO_PROFILING_API ReportNodeTraverser
    {
    public:
        // called for each node
        // if it returns false, it breaks the traversal
        virtual bool Traverse(ReportNode& node) = 0;

        // called when moving the traversal from a parent to a child
        virtual void Down() {};

        // called when moving the traversal up a level
        virtual void Up() {};

        virtual ~ReportNodeTraverser() {};
    };
} } // namespace chobo.profiling
