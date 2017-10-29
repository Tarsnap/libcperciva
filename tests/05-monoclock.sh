#!/bin/sh

### Constants
test_output="${s_basename}-stdout.txt"

### Actual command
scenario_cmd() {
	setup_check_variables
	cd ${scriptdir}/monoclock && \
	    ./test_monoclock 1> ${test_output}
	echo $? > ${c_exitfile}
}
