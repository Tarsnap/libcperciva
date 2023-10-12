#!/bin/sh

### Constants
c_valgrind_min=1
test_output="${s_basename}-stdout.txt"

### Actual command
scenario_cmd() {
	cd "${scriptdir}/sha1" || exit

	setup_check "test_sha1"
	${c_valgrind_cmd}			\
	    ./test_sha1 -x 1> "${test_output}"
	echo "$?" > "${c_exitfile}"
}
