#!/bin/sh

### Constants
c_valgrind_min=1
test_output="${s_basename}-stdout.txt"

### Actual command
scenario_cmd() {
	cd "${scriptdir}/sha256" || exit

	setup_check_variables "test_sha256"
	${c_valgrind_cmd}			\
	    ./test_sha256 -x 1> "${test_output}"
	echo "$?" > "${c_exitfile}"
}
