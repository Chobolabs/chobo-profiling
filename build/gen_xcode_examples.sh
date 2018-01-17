#/bin/bash

mkdir -p gen/osx
cd ge/osx
cmake ../../.. -G "Xcode" -DCHOBO_PROFILING_SHARED=1 -DCHOBO_PROFILING_BUILD_EXAMPLES=1
cd ../..
