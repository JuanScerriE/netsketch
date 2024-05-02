#!/bin/sh

# we shall assume that four threads are present on the host machine

cmake -DDUMPJSON=ON -S . -B build && scripts/copy_compile_commands.sh && make -C build -j 4
