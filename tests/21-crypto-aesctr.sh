#!/bin/sh

### Constants
c_valgrind_min=1
test_output="${s_basename}-stdout.txt"

### Actual command
scenario_cmd() {
	cd ${scriptdir}/crypto_aesctr || exit

	setup_check_variables "test_crypto_aesctr"
	${c_valgrind_cmd}			\
	    ./test_crypto_aesctr -x 1> ${test_output}
	echo "$?" > ${c_exitfile}
}
