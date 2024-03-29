#!/bin/sh

### Constants
c_valgrind_min=1
test_output="${s_basename}-stdout.txt"

### Actual command
scenario_cmd() {
	cd "${scriptdir}/elasticarray" || exit

	setup_check "test_elasticarray"
	${c_valgrind_cmd}			\
	    ./test_elasticarray -x 1> "${test_output}"
	echo "$?" > "${c_exitfile}"
}
