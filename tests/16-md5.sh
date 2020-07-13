#!/bin/sh

### Constants
c_valgrind_min=1
test_output="${s_basename}-stdout.txt"

### Actual command
scenario_cmd() {
	cd ${scriptdir}/md5

	setup_check_variables "test_md5"
	${c_valgrind_cmd}			\
	    ./test_md5 -x 1> ${test_output}
	echo "$?" > ${c_exitfile}
}
