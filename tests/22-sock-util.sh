#!/bin/sh

### Constants
c_valgrind_min=1
test_stderr="${s_basename}-stderr.txt"

### Actual command
scenario_cmd() {
	cd "${scriptdir}/sock_util" || exit

	setup_check_variables "test_sock_util"
	${c_valgrind_cmd}			\
	    ./test_sock_util 2> "${test_stderr}"
	echo "$?" > "${c_exitfile}"
}
