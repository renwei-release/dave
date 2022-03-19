#!/bin/sh

prefix=/dave/tools/jemalloc
exec_prefix=/dave/tools/jemalloc
libdir=${exec_prefix}/lib

LD_PRELOAD=${libdir}/libjemalloc.so.2
export LD_PRELOAD
exec "$@"
