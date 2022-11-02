#!/bin/bash

### Set up game directories & executable permissions
cd /kinslayer
mkdir -p lib/misc

### Set up core dump filename pattern - the defaults on some systems (ex: Ubuntu)
### don't work for us, so we'll need to set it explicitly. Note that this operation
### does require privileged access when running the container, otherwise attepting
### to modify this file will fail.
echo "core.%p.%t" > /proc/sys/kernel/core_pattern
make clean -C /kinslayer/src
make -C /kinslayer/src -j"$GCC_THREADS"
ldconfig
./bin/kinslayer
