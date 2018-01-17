#define CHOBO_PROFILING_ON 1
#include <chobo/profiling/internal/high_res_clock.h>
#include <chobo/Profiling.h>
#include <chobo/profiling/ProfilingManager.h>
#include <chobo/profiling/Profiler.h>
#include <chobo/profiling/ReportAggregatorPolicy.h>
#include <chobo/profiling/Report.h>
#include <chobo/profiling/ReportNode.h>

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"

TEST_SUITE_BEGIN("chobo-profiling");

TEST_CASE("clock")
{
    auto start = chobo::high_res_clock::now();
    auto dur = chobo::high_res_clock::now() - start;
    CHECK(std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count() == 0);

    chobo::test::this_thread_sleep_for_ns(10);
    dur = chobo::high_res_clock::now() - start;
    CHECK(std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count() == 10);

    chobo::test::this_thread_sleep_for(std::chrono::milliseconds(2));
    dur = chobo::high_res_clock::now() - start;
    CHECK(std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count() == 2000010);
}
namespace chobo {
namespace profiling {
bool operator==(const chobo::profiling::ProfilingData& lhs, const chobo::profiling::ProfilingData& rhs)
{
    return lhs.time == rhs.time
        && lhs.timesEntered == rhs.timesEntered
        && lhs.allocations == rhs.allocations
        && lhs.allocatedMemory == rhs.allocatedMemory
        && lhs.deallocations == rhs.deallocations
        && lhs.callStackLevel == rhs.callStackLevel
        && lhs.extraData == rhs.extraData;
}
}}

struct Test
{
    void sleep(int ms)
    {
        CHOBO_PROFILE_FUNC_SPLIT();

        chobo::test::this_thread_sleep_for(std::chrono::milliseconds(ms)); // ms * 1000000
    }

    void sleep(float sec)
    {
        chobo::test::this_thread_sleep_for(std::chrono::microseconds(int(sec * 1000000)));
    }
};

void bar(Test& t)
{
    const char* label = "bar";
    CHOBO_PROFILE_SECTION_ENTER(label);
    t.sleep(3); // 3000000
    t.sleep(2); // 2000000
    CHOBO_PROFILE_SECTION_LEAVE(label);
    // 5000000
}

void foo(Test& t)
{
    CHOBO_PROFILE_FUNC();
    t.sleep(0.006f); // 6000000
    bar(t); // 5000000
    // 11000000
}

TEST_CASE("basic")
{
    Test test;
    
    auto& profiler = chobo::profiling::ProfilingManager::GetInstance().GetLocalProfiler();
    CHECK(profiler.GetName().empty());
    CHECK(profiler.GetRootNode().GetChildren().size() == 0);

    for (int i = 0; i < 5; ++i)
    {
        CHOBO_PROFILE_SCOPE("main loop"); // 2
        foo(test); // 11000000
        bar(test); // 5000000
        test.sleep(0.01f); // 10000000
        // 26000000
    }

    auto& proot = profiler.GetRootNode();

    CHECK(&profiler.GetCurrentNode() == &proot);
    CHECK(profiler.GetName().empty());
    REQUIRE(proot.GetChildren().size() == 1);
    auto pmainLoop = proot.GetChildren().front();
    CHECK(pmainLoop->GetChildren().size() == 2);
    auto pmainLoopData = pmainLoop->GetProfilingData();
    CHECK(pmainLoopData.timesEntered == 5);
    CHECK(pmainLoopData.time == 130000000);
    CHECK(pmainLoopData.callStackLevel == 1);
    
    auto pmainLoop_foo = pmainLoop->GetChildren().front();
    auto pmainLoop_fooData = pmainLoop_foo->GetProfilingData();
    CHECK(pmainLoop_fooData.timesEntered == 5);
    CHECK(pmainLoop_fooData.time == 55000000);
    CHECK(pmainLoop_fooData.callStackLevel == 2);

    CHECK(pmainLoop_foo->GetChildren().size() == 1);
    auto pmainLoop_foo_bar = pmainLoop_foo->GetChildren().front();
    auto pmainLoop_foo_barData = pmainLoop_foo_bar->GetProfilingData();
    CHECK(pmainLoop_foo_barData.timesEntered == 5);
    CHECK(pmainLoop_foo_barData.time == 25000000);
    CHECK(pmainLoop_foo_barData.callStackLevel == 3);

    auto pmainLoop_bar = pmainLoop->GetChildren().back();
    auto pmainLoop_barData = pmainLoop_bar->GetProfilingData();
    CHECK(pmainLoop_barData.timesEntered == 5);
    CHECK(pmainLoop_barData.time == 25000000);
    CHECK(pmainLoop_barData.callStackLevel == 2);
    
    CHECK(pmainLoop_bar->GetChildren().size() == 1);
    auto pmainLoop_bar_sleep = pmainLoop_bar->GetChildren().back();
    auto pmainLoop_bar_sleepData = pmainLoop_bar_sleep->GetProfilingData();
    CHECK(pmainLoop_bar_sleepData.timesEntered == 10);
    CHECK(pmainLoop_bar_sleepData.time == 25000000);
    CHECK(pmainLoop_bar_sleepData.callStackLevel == 3);

    CHECK(pmainLoop_bar_sleep->GetChildren().size() == 10);

    auto pmainLoop_bar_sleep_c1 = pmainLoop_bar_sleep->GetChildren().at(1);
    auto pmainLoop_bar_sleep_c1Data = pmainLoop_bar_sleep_c1->GetProfilingData();
    CHECK(pmainLoop_bar_sleep_c1Data.timesEntered == 1);
    CHECK(pmainLoop_bar_sleep_c1Data.time == 2000000);
    CHECK(pmainLoop_bar_sleep_c1Data.callStackLevel == 4);
    CHECK(pmainLoop_bar_sleep_c1->GetChildren().empty());

    auto& report = profiler.CalculateRootReport();

    auto& roots = report.GetRoots();
    REQUIRE(roots.size() == 1);
    auto rroot = roots.front();
    REQUIRE(rroot->GetChildren().size() == 1);
    auto rmainLoop = rroot->GetChildren().front();
    CHECK(rmainLoop->GetSectionLabel() == "main loop");
    CHECK(rmainLoop->GetChildren().size() == 2);
    CHECK(rmainLoop->GetProfilingData() == pmainLoopData);

    auto rmainLoop_foo = rmainLoop->GetChildren().front();
    CHECK(rmainLoop_foo->GetSectionLabel().find("foo") != std::string::npos);
    CHECK(rmainLoop_foo->GetProfilingData() == pmainLoop_fooData);

    CHECK(rmainLoop_foo->GetChildren().size() == 1);
    auto rmainLoop_foo_bar = rmainLoop_foo->GetChildren().front();
    CHECK(rmainLoop_foo_bar->GetSectionLabel() == "bar");
    CHECK(rmainLoop_foo_bar->GetProfilingData() == pmainLoop_foo_barData);
    
    auto rmainLoop_bar = rmainLoop->GetChildren().back();
    CHECK(rmainLoop_bar->GetSectionLabel() == "bar");
    CHECK(rmainLoop_bar->GetProfilingData() == pmainLoop_barData);

    auto rmainLoop_bar_sleep = rmainLoop_bar->GetChildren().front();
    CHECK(rmainLoop_bar_sleep->GetSectionLabel().find("Test") != std::string::npos);
    CHECK(rmainLoop_bar_sleep->GetSectionLabel().find("sleep") != std::string::npos);
    CHECK(rmainLoop_bar_sleep->GetProfilingData() == pmainLoop_bar_sleepData);

    profiler.Reset();
    CHECK(profiler.GetRootNode().GetChildren().size() == 0);
}

int main(int argc, char** argv)
{
    CHOBO_PROFILE_CURRENT_THREAD();

    doctest::Context context; // initialize

    context.applyCommandLine(argc, argv);

    return context.run();
}
