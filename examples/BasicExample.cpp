#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <algorithm>

#if defined(__MINGW32__)
#include <Windows.h>
namespace std
{
    namespace this_thread
    {
        template <class Rep, class Period>
        void sleep_for(const std::chrono::duration<Rep, Period>& sleep_duration)
        {
            Sleep(chrono::duration_cast<chrono::milliseconds>(sleep_duration).count());
        }
    }
}
#endif

#define CHOBO_PROFILING_ON 1
#include <chobo/Profiling.h>
#include <chobo/profiling/ProfilingManager.h>
#include <chobo/profiling/ReportAggregatorPolicy.h>
#include <chobo/profiling/Profiler.h>
#include <chobo/profiling/Report.h>
#include <chobo/profiling/SimpleReportTraverser.h>
#include <chobo/profiling/ReportNode.h>
#include <chobo/profiling/ProfilerPauseSentry.h>
#include <chobo/profiling/aggregators/LocalData.h>
#include <chobo/profiling/aggregators/Sum.h>
#include <chobo/profiling/aggregators/MovingAverage.h>

using namespace std;
using namespace std::chrono;
using namespace chobo::profiling;

struct Test
{
    void sleep(int ms)
    {
        CHOBO_PROFILE_FUNC_SPLIT();

        vector<bool> adsad;
        map<int, int> test;

        this_thread::sleep_for(milliseconds(ms));
    }

    void sleep(float sec)
    {
        this_thread::sleep_for(microseconds(int(sec * 1000000)));
    }
};

void bar(Test& t)
{
    const char* label = "bar";
    CHOBO_PROFILE_SECTION_ENTER(label);
    t.sleep(3);
    t.sleep(2);

    vector<int> x;
    x.push_back(1);
    x.push_back(2);

    auto a = make_shared<vector<int>>(10, 1);
    CHOBO_PROFILE_SECTION_LEAVE(label);
}

void foo(Test& t)
{
    CHOBO_PROFILE_FUNC();
    t.sleep(0.006f);
    bar(t);
}

class TestPolicy : public ReportAggregatorPolicy
{
public:
    TestPolicy()
    {
        static std::vector<size_t> ids{
            aggregators::Sum::id,
            aggregators::LocalData::id,
            aggregators::MovingAverage::id
        };
        
        auto size = *max_element(ids.begin(), ids.end()) + 1;
        
        m_aggregatorIds.resize(size, size_t(-1));
        for (size_t i = 0; i < ids.size(); ++i)
        {
            m_aggregatorIds[ids[i]] = i;
        }
    }

    virtual std::vector<std::shared_ptr<ReportAggregator>> GetAggregatorsForNewNode(const ReportNode& node) const override
    {
        std::vector<std::shared_ptr<ReportAggregator>> aggs(3);
        aggs[0].reset(new aggregators::Sum(node));
        aggs[1].reset(new aggregators::LocalData(node));
        aggs[2].reset(new aggregators::MovingAverage(node, 3));
        return aggs;
    }
};

class ReportTraverser : public SimpleReportTraverser
{
public:
    ReportTraverser()
        : SimpleReportTraverser(cout)
    {}

    bool Traverse(ReportNode& node) override
    {
        auto sum = node.GetAggregator<aggregators::Sum>();        
        auto& data = sum->GetData();

        auto local = node.GetAggregator<aggregators::LocalData>();
        auto utime = local->GetTotalUnprofiledTime();

        auto& movingavg = node.GetAggregator<aggregators::MovingAverage>()->GetAverage();

        m_out << m_indent
            << node.GetSectionLabel() << ": "
            << n2m(data.time) << " ms, "
            << "(u: " << n2m(utime) << " ms), "
            << data.timesEntered << " calls";

        if (movingavg.timesEntered != 0)
        {
            m_out << ", "
                << n2m(movingavg.GetMeanTime()) << " ms ("
                << n2m(utime / data.timesEntered) << " ms) each";
        }

        m_out << ". a:" << movingavg.allocations << " (" << movingavg.allocatedMemory << ") d: " << movingavg.deallocations << "(local: " << local->GetLocalAllocations() << ")";

        m_out << endl;

        return true;
    }
};

struct TestMem
{
    int foo;
    float bar;
};

int main()
{
    ProfilingManager::GetInstance().BeginProfilingForCurrentThread();
    auto& profiler = ProfilingManager::GetInstance().GetLocalProfiler();

    CHOBO_MEMORY_PROFILE_SCOPE()

    Test test;

    auto policy = make_shared<TestPolicy>();
    Report& report = profiler.CalculateRootReport(policy);
    profiler.Clear();

    for (size_t i = 0; i < 30; ++i)
    {
        string p2std = "test";
        string pFromStd = "Asdasd";
        pFromStd = p2std;

        CHOBO_PROFILE_SCOPE("main loop");
        foo(test);
        bar(test);
        test.sleep(0.01f);

        auto p = new TestMem[20];
        memset(p, 0, sizeof(TestMem) * 20);
        delete[] p;

        {
            CHOBO_PROFILER_PAUSE_SENTRY(profiler);
            profiler.CalculateRootReport();
        }
        profiler.Clear();
    }

    SimpleReportTraverser t(cout);
    //ReportTraverser t;
    report.UpdateFlatNodes();
    
    t.TraverseReport(report);
    cout << endl << endl;
    report.TraverseFlatData(t);

    cout << endl << endl;

    auto clonePolicy = make_shared<TestPolicy>();
    auto clone = report.Clone(clonePolicy);

    t.TraverseReport(*clone);
    cout << endl << endl;
    clone->TraverseFlatData(t);

    delete clone;

    return 0;
}
