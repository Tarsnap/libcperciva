#!/bin/sh

### Constants
c_valgrind_min=1
test_output="${s_basename}-stdout.txt"

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
}
