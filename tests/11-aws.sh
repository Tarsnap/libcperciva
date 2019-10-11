#!/bin/sh

### Constants
c_valgrind_min=1
test_output="${s_basename}-stderr.txt"

### Actual command
scenario_cmd() {
	setup_check_variables
	cd ${scriptdir}/aws/ && ${c_valgrind_cmd} \
	    ./test_aws 2> ${test_output}
	echo "$?" > ${c_exitfile}

	setup_check_variables
	cmp ${scriptdir}/aws/test_aws.good ${test_output}
	echo $? > ${c_exitfile}
}
