#!/bin/sh

# we shall assume that four threads are present on the host machine

cmake -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_CXX_FLAGS="-fsanitize=thread" -S . -B build && scripts/copy_compile_commands.sh && make -C build -j 4
