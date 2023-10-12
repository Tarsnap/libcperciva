#!/bin/sh

### Constants
c_valgrind_min=1
test_output="${s_basename}-stdout.txt"
longjmp_output="${s_basename}-longjmp-stderr.txt"

### Actual command
scenario_cmd() {
	cd "${scriptdir}/getopt" || exit

	setup_check "test_getopt.sh"
	# Pass ${c_valgrind_cmd} to the script
	c_valgrind_cmd=${c_valgrind_cmd}	\
	    ./test_getopt.sh 1> "${test_output}"
	echo "$?" > "${c_exitfile}"

	# Check for undesired assignment (i.e. the compiler
	# putting a=1 before a siglongjmp).
	cd "${scriptdir}/getopt-longjmp" || exit

	setup_check "test_getopt_longjmp"
	c_valgrind_cmd=${c_valgrind_cmd}	\
	    ./test_getopt_longjmp 2> "${longjmp_output}"
	echo "$?" > "${c_exitfile}"
}
