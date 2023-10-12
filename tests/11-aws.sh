#!/bin/sh

### Constants
c_valgrind_min=1
test_output="${s_basename}-stderr.txt"

### Actual command
scenario_cmd() {
	cd "${scriptdir}/aws" || exit

	setup_check "test_aws"
	${c_valgrind_cmd}			\
	    ./test_aws 2> "${test_output}"
	echo "$?" > "${c_exitfile}"

	setup_check "test_aws output against reference"
	cmp -s "${scriptdir}/aws/test_aws.good" "${test_output}"
	echo "$?" > "${c_exitfile}"
}
