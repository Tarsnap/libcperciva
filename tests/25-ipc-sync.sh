#!/bin/sh

### Constants
c_valgrind_min=1
num_tests=8

### Actual command
scenario_cmd() {
	cd "${scriptdir}/ipc_sync" || exit

	i=0
	while [ "${i}" -lt "${num_tests}" ]; do
		setup_check "test_ipc_sync ${i}"
		# Special handling for multiple forks.
		c_valgrind_cmd=$(valgrind_setup "valgrind-parent")
		${c_valgrind_cmd} ./test_ipc_sync "${i}"		\
			> "${s_basename}-${c_count_str}.stdout"
		echo "$?" > "${c_exitfile}"
		i=$((i + 1))
	done
}
