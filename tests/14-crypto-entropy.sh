#!/bin/sh

### Constants
c_valgrind_min=1

### Actual command
scenario_cmd() {
	cd ${scriptdir}/crypto_entropy

	setup_check_variables
	${c_valgrind_cmd}			\
	    ./test_crypto_entropy > /dev/null
	echo "$?" > ${c_exitfile}
}
