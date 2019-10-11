#!/bin/sh

### Constants
c_valgrind_min=1
test_output="${s_basename}-stdout.txt"

### Actual command
scenario_cmd() {
	cd ${scriptdir}/crc32

	setup_check_variables
	${c_valgrind_cmd}			\
	    ./test_crc32 -x 1> ${test_output}
	echo "$?" > ${c_exitfile}
}
