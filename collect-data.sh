#!/bin/sh

echo "num_of_clients, iterations, interval_sec, average_of_completion_times_microsec, average_of_average_network_input_times" >data.csv

configs=(
	"5 10 0.125"
	"5 10 0.25"
	"5 10 0.5"
	"5 10 1"
	"10 10 0.125"
	"10 10 0.25"
	"10 10 0.5"
	"10 20 0.125"
	"10 20 0.25"
	"10 20 0.5"
	"50 10 0.125"
	"50 10 0.25"
	"50 10 0.5"
	"100 10 0.125"
	"100 10 0.25"
	"100 10 0.5"
	"500 10 0.125"
	"500 10 0.25"
    # NOTE: these take way to long because they fail (default
    # server timeout is 10 minutes)
	# "1000 5 0.125"
	# "1000 5 0.25"
	# "1000 10 0.125"
)

# iterate over each row of the nested array
for ((i = 0; i < ${#configs[@]}; i++)); do
	config=(${configs[i]})

	clients=${config[0]}
	iterations=${config[1]}
	interval=${config[2]}

	echo "current config:"
	echo "  clients=$clients"
	echo "  iterations=$iterations"
	echo "  interval=$interval"

	./build/src/netsketch_server 1>/dev/null 2>/dev/null &

	server_pid=$!
	echo "server_pid=$server_pid"

	times=$(./stress-server.sh $clients $iterations $interval)

	kill -INT $server_pid

	if [ $? == 0 ]; then
		completion_times=$(echo "$times" | grep "moving average of \"full test client run\":" | sed 's/.*"full test client run": //g' | sed 's/µs//g')

		n=0
		sum=0

		for time in $completion_times; do
			sum=$(($sum + $time))
			n=$((n + 1))
		done

		average_of_completion_times=$((sum / n))

		average_network_input_times=$(echo "$times" | grep "moving average of \"reading input from network\":" | sed 's/.*"reading input from network": //g' | sed 's/µs//g')

		n=0
		sum=0

		for time in $average_network_input_times; do
			sum=$(($sum + $time))
			n=$((n + 1))
		done
		average_of_average_network_input_times=$((sum / n))

		echo "$clients, $iterations, $interval, $average_of_completion_times, $average_of_average_network_input_times" >>data.csv
		echo "appended info to file"

		echo "waiting on server ($server_pid) to shutdown..."
		wait $server_pid
		echo "server ($server_pid) shutdown..."
	fi
done
