#
# chobo-profiling
# Copyright (c) 2015-2018 Chobolabs Inc.
# http://www.chobolabs.com/
#
# Distributed under the MIT Software License
# See accompanying file LICENSE.txt or copy at
# https://opensource.org/licenses/MIT
#
cmake_minimum_required(VERSION 3.2.2)

project(chobo-profiling)

option(CHOBO_PROFILING_SHARED "Build chobo-profiling as a shared library" OFF)
option(CHOBO_PROFILING_BUILD_EXAMPLES "Build chobo-profiling examples" OFF)
option(CHOBO_PROFILING_BUILD_TESTS "Build chobo-profiling tests" OFF)

if(NOT COMMAND chobo_source_group)
    # we're not in choboland so include macros
    include(build/cmake/macros.cmake)
endif()

if(NOT CHOBO_SHL_PATH)
    # use chobo shl from submodule
    set(CHOBO_SHL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/chobo-shl/include)
endif()

set(inc ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(inc_path ${inc}/chobo/profiling)
set(src_path ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(sources)
chobo_source_group(profiling sources
    ${inc}/chobo/Profiling.h
    ${inc_path}/Config.h
    ${src_path}/Internal.h
    ${src_path}/MemoryProfiling.cpp
    ${inc_path}/MemoryProfiling.h
    ${src_path}/Profiler.cpp
    ${inc_path}/Profiler.h
    ${src_path}/ProfilerNode.cpp
    ${inc_path}/ProfilerNode.h
    ${inc_path}/ProfilerNodeTraverser.h
    ${src_path}/ProfilerPauseSentry.cpp
    ${inc_path}/ProfilerPauseSentry.h
    ${inc_path}/ProfileSentry.h
    ${inc_path}/ProfilingData.h
    ${src_path}/ProfilingManager.cpp
    ${inc_path}/ProfilingManager.h
    ${src_path}/Report.cpp
    ${inc_path}/Report.h
    ${inc_path}/ReportAggregatorPolicy.h
    ${src_path}/ReportNode.cpp
    ${inc_path}/ReportNode.h
    ${inc_path}/ReportNodeTraverser.h
    ${src_path}/Section.cpp
    ${inc_path}/Section.h
    ${src_path}/SimpleReportTraverser.cpp
    ${inc_path}/SimpleReportTraverser.h
    ${src_path}/Tag.cpp
    ${inc_path}/Tag.h
)

chobo_source_group("profiling\\\\aggregators" sources
    ${src_path}/aggregators/ReportAggregator.cpp
    ${inc_path}/aggregators/ReportAggregator.h
    ${src_path}/aggregators/MovingAverage.cpp
    ${inc_path}/aggregators/MovingAverage.h
    ${src_path}/aggregators/Sum.cpp
    ${inc_path}/aggregators/Sum.h
    ${src_path}/aggregators/LocalData.cpp
    ${inc_path}/aggregators/LocalData.h
)

chobo_source_group("profiling\\\\internal" sources
    ${src_path}/internal/high_res_clock.cpp
    ${inc_path}/internal/high_res_clock.h
    ${inc_path}/internal/thread.h
)

if(CHOBO_PROFILING_SHARED)
    add_library(chobo-profiling SHARED
        ${sources}
    )

    target_compile_definitions(chobo-profiling PUBLIC
        -DCHOBO_PROFILING_DYN_LINK
    )

    if(MSVC)
        target_compile_options(chobo-profiling PUBLIC
            "/wd4251"
        )
    endif()

    if (CMAKE_HOST_APPLE)
        set_target_properties(chobo-profiling PROPERTIES MACOSX_RPATH ON)
    endif()
else()
    add_library(chobo-profiling STATIC
        ${sources}
    )
endif()

set_target_properties(chobo-profiling PROPERTIES FOLDER chobo)

target_compile_features(chobo-profiling PUBLIC
    cxx_alias_templates
    cxx_auto_type
    cxx_decltype
    cxx_delegating_constructors
    cxx_final
    cxx_lambdas
    cxx_override
    cxx_range_for
    cxx_right_angle_brackets
    cxx_rvalue_references
    cxx_static_assert
)

target_include_directories(chobo-profiling PUBLIC
    ${inc}
    ${CHOBO_SHL_PATH}
)

if(CHOBO_PROFILING_BUILD_EXAMPLES)
    add_executable(basic-example examples/BasicExample.cpp)
    target_link_libraries(basic-example chobo-profiling)

    add_executable(mt-example examples/MTExample.cpp)
    target_link_libraries(mt-example chobo-profiling)
endif()

if(CHOBO_PROFILING_BUILD_TESTS)
    enable_testing()
    target_compile_definitions(chobo-profiling PUBLIC
        -DCHOBO_PROFILING_TEST=1
    )

    add_executable(test_basic test/BasicTest.cpp)
    target_link_libraries(test_basic chobo-profiling)
    add_test(test_basic test_basic)
endif()
