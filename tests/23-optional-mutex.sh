#!/bin/sh

### Constants
c_valgrind_min=1

### Actual command
scenario_cmd() {
	setup_check "test_optional_pthread_no"
	cd "${scriptdir}/optional_mutex/normal" || exit
	${c_valgrind_cmd} ./test_optional_pthread_no
	echo "$?" > "${c_exitfile}"

	setup_check "test_optional_pthread_yes"
	cd "${scriptdir}/optional_mutex/pthread" || exit
	${c_valgrind_cmd} ./test_optional_pthread_yes
	echo "$?" > "${c_exitfile}"
}
