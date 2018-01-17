#include <iostream>
#include <thread>
#include <functional>

#define CHOBO_PROFILING_ON 1
#include <chobo/Profiling.h>
#include <chobo/profiling/ProfilingManager.h>
#include <chobo/profiling/SimpleReportTraverser.h>
#include <chobo/profiling/Profiler.h>

using namespace std::chrono;
using namespace chobo::profiling;

using namespace std;

void common(int ms)
{
    CHOBO_PROFILE_FUNC_MT();
    this_thread::sleep_for(milliseconds(ms));
}

void rcommon(int ms)
{
    CHOBO_PROFILE_FUNC_SPLIT_MT();
    this_thread::sleep_for(milliseconds(ms));
}

struct ThreadB
{
    void foo(int ms)
    {
        CHOBO_PROFILE_FUNC();
        this_thread::sleep_for(milliseconds(ms));
    }

    void bar()
    {
        CHOBO_PROFILE_FUNC();
        this_thread::sleep_for(milliseconds(5));
    }

    void Execute()
    {
        CHOBO_PROFILE_CURRENT_THREAD();
        profiler = &ProfilingManager::GetInstance().GetLocalProfiler();

        for (int i = 0; i < 20; ++i)
        {
            CHOBO_PROFILE_SCOPE("B");
            foo(10);
            bar();
            foo(20);
            common(15);

            if (rand() % 3 == 0)
                rcommon(30);
        }
    }

    Profiler* profiler;
};

struct ThreadA
{
    void foo(int ms)
    {
        CHOBO_PROFILE_FUNC();
        this_thread::sleep_for(milliseconds(ms));
    }

    void bar()
    {
        CHOBO_PROFILE_FUNC();
        this_thread::sleep_for(milliseconds(10));
    }

    void Execute()
    {
        //CHOBO_RECYCLE_NAMED_PROFILER("Thread AAA");
        CHOBO_ATTACH_NAMED_PROFILER_TO_CURRENT_THREAD("AAA");

        profiler = &ProfilingManager::GetInstance().GetLocalProfiler();

        for (int i = 0; i < 30; ++i)
        {
            CHOBO_PROFILE_SCOPE("A");
            foo(15);
            bar();
            foo(35);
            common(5);

            if (rand() % 3 == 0)
                rcommon(15);
        }
    }

    Profiler* profiler;
};

int main()
{
    ThreadA a;
    thread t1(bind(&ThreadA::Execute, &a));
    
    ThreadB b;
    b.Execute();

    t1.join();

    thread t2(bind(&ThreadA::Execute, &a));
    t2.join();

    SimpleReportTraverser t(cout);
    t.TraverseProfiler(*a.profiler);
    t.TraverseProfiler(*b.profiler);
}
