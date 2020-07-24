#!/bin/sh

### Constants
c_valgrind_min=1
test_stderr="${s_basename}-stderr.txt"


### Actual command
scenario_cmd() {
	cd ${scriptdir}/daemonize || exit 1

	# Run the binary.
	setup_check_variables "test_daemonize"
	${c_valgrind_cmd}			\
	    ./test_daemonize /dev/null 2> ${test_stderr}
	echo "$?" > ${c_exitfile}

	# Check that we only have two lines of output (i.e. no
	# multiple lines due to buffered output before fork()).
	setup_check_variables "test_daemonize output"
	test "$(wc -l < "${test_stderr}")" -eq "2"
	echo "$?" > ${c_exitfile}
}
