#!/bin/sh

# set -x

echo "num_of_clients, iterations, interval_sec, average_for_completion_nanosec" > data.csv

configs=(
    "4 10 0.5"
    "10 20 1"
    "30 20 1"
    "30 20 0.5"
    "100 20 0.5"
    "100 40 0.2"
    "500 20 0.25"
    "1000 10 1"
    "1000 10 0.4"
)

# Iterate over each row of the nested array
for ((i = 0; i < ${#configs[@]}; i++))
do
    config=(${configs[i]})

    clients=${config[0]}
    iterations=${config[1]}
    interval=${config[2]}

    echo "current config:"
    echo "  clients=$clients"
    echo "  iterations=$iterations"
    echo "  interval=$interval"

    ./build/src/netsketch_server --time-out 0.5 1> /dev/null 2> /dev/null &

    server_pid=$!

    echo "server_pid=$server_pid"

    times=$(./stress-server.sh $clients $iterations $interval | grep "full test client run" | sed 's/.*"full test client run": //g' | sed 's/µs//g')

    kill -INT $server_pid

    n=0

    sum=0

    for time in $times
    do
        sum=$(($sum + $time))

        n=$((n+1))
    done

    average=$((sum/n))

    echo "$clients, $iterations, $interval, $average" >> data.csv

    echo "appended info to file"
done
