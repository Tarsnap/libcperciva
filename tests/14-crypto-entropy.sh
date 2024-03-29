#!/bin/sh

### Constants
c_valgrind_min=1

### Actual command
scenario_cmd() {
	cd "${scriptdir}/crypto_entropy" || exit

	setup_check "test_crypto_entropy"
	${c_valgrind_cmd}			\
	    ./test_crypto_entropy > /dev/null
	echo "$?" > "${c_exitfile}"
}
