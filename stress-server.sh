#!/bin/sh

# set -x

# --expected-responses $(($1*$2))

max=$1

echo "stress-server.sh: starting the test clients..."
for i in `seq 0 $max`
do
    build/src/netsketch_test_client --nickname juan$i --iterations $2 --interval $3  &

    pids[$i]=$!
done

echo "stress-server.sh: waiting for all test clients to finish..."
for pid in ${pids[*]}
do
    wait $pid
done
