#!/bin/sh

### Constants
c_valgrind_min=1
test_output="${s_basename}-stdout.txt"
test_emptyloop_output="${s_basename}-stdout-emptyloop.txt"

### Actual command
scenario_cmd() {
	cd ${scriptdir}/events

	setup_check_variables
	${c_valgrind_cmd}			\
	    ./test_events 1> ${test_output}
	echo "$?" > ${c_exitfile}

	setup_check_variables
	cmp -s ${scriptdir}/events/test-events.good ${test_output}
	echo "$?" > ${c_exitfile}

	setup_check_variables
	# Run a loop without any events
	${c_valgrind_cmd}			\
		./test_events 1 > ${test_emptyloop_output} &
	pid=$!
	sleep 1
	kill -s USR1 ${pid}
	# Extra delay for valgrind to finish exiting
	if [ -n "${c_valgrind_cmd}" ]; then
		sleep 1
	fi

	# Compare with good values
	setup_check_variables
	cmp -s test-empty-events.good ${test_emptyloop_output}
	echo "$?" > ${c_exitfile}
}
