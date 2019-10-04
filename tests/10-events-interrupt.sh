#!/bin/sh

### Constants
c_valgrind_min=1
test_output="${s_basename}-stdout.txt"
test_network_output="${s_basename}-network-stdout.txt"

### Actual command
scenario_cmd() {
	# Non-network events
	setup_check_variables
	cd ${scriptdir}/events && ${c_valgrind_cmd}			\
	    ./test_events 1> ${test_output}
	echo $? > ${c_exitfile}

	setup_check_variables
	cmp ${scriptdir}/events/test-events.good ${test_output}
	echo $? > ${c_exitfile}

	# Network events (uses server & client binaries).
	setup_check_variables
	cd ${scriptdir}/events &&					\
	    ./test-events-network.sh 1> ${test_network_output}
	echo $? > ${c_exitfile}

	setup_check_variables
	cmp ${scriptdir}/events/test-events-network.good ${test_network_output}
	echo $? > ${c_exitfile}
}
