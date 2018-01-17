#
# chobo-cmake
#
# Collection of CMake macros used at Chobolabs
#
# MIT License:
# Copyright(c) 2015-2018 Chobolabs Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files(the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and / or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions :
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
# OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
# WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# chobo_msg
#
# Wraps the message() command by adding the text "Chobo CMake" to the message.
# It's used to more easily identify errors and messages coming from our code.
macro(chobo_msg type msg)
    message(${type} "Chobo CMake: ${msg}")
endmacro(chobo_msg)

# chobo_source_group
#
# Defines a group of source files, while also appending them to a list (to be used with add_executable or add_library)
# Usage
# chobo_source_group("Group" list file1 file2 ... fileN)
#
macro(chobo_source_group GROUP_NAME SRC_LIST)
    source_group(${GROUP_NAME} FILES ${ARGN})

    foreach(filename ${ARGN})
        list(APPEND ${SRC_LIST} ${filename})
    endforeach()
endmacro(chobo_source_group)
