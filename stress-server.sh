#!/bin/sh

set -x

max=$1

for i in `seq 0 $max`
do
    build/src/netsketch_test_client --nickname juan$i --iterations $2 --interval $3 &
done
