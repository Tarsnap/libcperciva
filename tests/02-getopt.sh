#!/bin/sh

### Constants
c_valgrind_min=1
test_output="${s_basename}-stdout.txt"

### Actual command
scenario_cmd() {
	cd ${scriptdir}/getopt

	setup_check_variables
	# Pass ${c_valgrind_cmd} to the script
	c_valgrind_cmd=${c_valgrind_cmd}	\
	    ./test_getopt.sh 1> ${test_output}
	echo "$?" > ${c_exitfile}
}
