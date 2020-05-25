#!/bin/bash

impl=$1
in_f=$2
out_f=$3

in_files="$in_f tests/libs/libuthread/uthread.c tests/libs/libuthread/queue.c tests/libs/libuthread/context.c"

if [ "$impl" = "WASMTIME_CONTS" ]; then
    extra_args="libs/continuations.c -DCONTEXT_IMPL=WASMTIME_CONTS"
    emcc_control $in_files $extra_args "$out_f.wat"
elif [ "$impl" = "WASMTIME_ASYNCIFY" ]; then
    echo "NOT YET IMPLEMENTED"
elif [ "$impl" = "NATIVE_SWAPCONTEXT" ]; then
    extra_args="-DCONTEXT_IMPL=NATIVE_SWAPCONTEXT -D_XOPEN_SOURCE -Wno-deprecated-declarations -O3"
    clang $in_files $extra_args -o $out_f.out
fi