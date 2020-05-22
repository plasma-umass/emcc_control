#!/bin/bash

impl=$1
in_f=$2
out_f=$3

in_files="$in_f tests/libs/libuthread/uthread.c tests/libs/libuthread/queue.c tests/libs/libuthread/context.c"

if [ "$impl" = "CONTS" ]; then
    extra_args="libs/continuations.c -DCONTEXT_IMPL=CONTS"
    ./emcc_control $in_files $extra_args "$out_f.wat"
elif [ "$impl" = "ASYNCIFY" ]; then
    echo "NOT YET IMPLEMENTED"
elif [ "$impl" = "SWAPCONTEXT" ]; then
    extra_args="-DCONTEXT_IMPL=SWAPCONTEXT -D_XOPEN_SOURCE -Wno-deprecated-declarations -O3"
    clang $in_files $extra_args -o $out_f.out
fi