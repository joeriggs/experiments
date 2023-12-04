#!/bin/bash

gcc -O0 -g breakpoint.c -o breakpoint -lrt
[ $? -ne 0 ] && echo "breakpoint build failed." && exit 1

gcc -O0 -g stack_trace.c -o stack_trace
[ $? -ne 0 ] && echo "stack_trace build failed." && exit 1

exit 0
