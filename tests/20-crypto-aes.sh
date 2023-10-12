#!/bin/sh

### Constants
c_valgrind_min=1
test_output="${s_basename}-stdout.txt"

### Actual command
scenario_cmd() {
	cd "${scriptdir}/crypto_aes" || exit

	setup_check "test_crypto_aes"
	${c_valgrind_cmd}			\
	    ./test_crypto_aes -x 1> "${test_output}"
	echo "$?" > "${c_exitfile}"
}
