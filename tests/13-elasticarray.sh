#!/bin/sh

### Constants
c_valgrind_min=1
test_output="${s_basename}-stdout.txt"

### Actual command
scenario_cmd() {
	cd ${scriptdir}/elasticarray

	setup_check_variables
	${c_valgrind_cmd}			\
	    ./test_elasticarray 1> ${test_output}
	echo "$?" > ${c_exitfile}
}
