@echo off
if not exist gen mkdir gen
cd gen
if not exist vs14test mkdir vs14test
cd vs14test
cmake ../../.. -G "Visual Studio 14 2015 Win64" -DCHOBO_PROFILING_SHARED=1 -DCHOBO_PROFILING_BUILD_TESTS=1
cd ..\..