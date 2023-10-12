#!/bin/sh

### Constants
c_valgrind_min=1
test_output="${s_basename}-stdout.txt"

### Actual command
scenario_cmd() {
	cd "${scriptdir}/humansize" || exit

	setup_check "test_humansize"
	${c_valgrind_cmd}			\
	    ./test_humansize 1> "${test_output}"
	echo "$?" > "${c_exitfile}"
}
