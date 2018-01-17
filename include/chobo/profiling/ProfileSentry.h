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

#include <cstddef>

namespace chobo { namespace profiling
{
    template <typename Sec>
    class ProfileSentry
    {
    public:
        ProfileSentry(Sec& section)
            : m_section(section)
        {
            m_section.Enter();
        }
        
        ~ProfileSentry()
        {
            m_section.Leave();
        }

    private:
        Sec& m_section;
    };

} } // namespace chobo.profiling
