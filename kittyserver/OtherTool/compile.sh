#!/bin/sh

make clean
make distclean
make proto
make res
make -j2
