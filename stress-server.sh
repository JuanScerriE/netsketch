#!/bin/sh

# set -x

max=$(($1 - 1))

echo "stress-server.sh: starting the test clients..."
for i in `seq 0 $max`
do
    build/src/netsketch_test_client --username juan$i --iterations $2 --interval $3 --expected-responses $(($1*$2)) &

    pids[$i]=$!
done

echo "stress-server.sh: waiting for all test clients to finish..."
for pid in ${pids[*]}
do
    wait $pid
done
