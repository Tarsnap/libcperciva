#!/bin/sh

### Constants
c_valgrind_min=1
spawning="${s_basename}-spawning.txt"
waited="${s_basename}-waited.txt"

### Actual command
scenario_cmd() {
	cd "${scriptdir}/fork_func" || exit

	setup_check "test_fork_func -x"
	# Special handling for multiple forks.
	c_valgrind_cmd=$(valgrind_setup "valgrind-parent")
	${c_valgrind_cmd} ./test_fork_func -x
	echo "$?" > "${c_exitfile}"

	setup_check "test_fork_func -e"
	# Special handling for multiple forks.
	c_valgrind_cmd=$(valgrind_setup "valgrind-parent")
	${c_valgrind_cmd} ./test_fork_func -e
	echo "$?" > "${c_exitfile}"

	setup_check "test_fork_func -c"
	# Special handling for multiple forks.
	c_valgrind_cmd=$(valgrind_setup "valgrind-parent")
	${c_valgrind_cmd} ./test_fork_func -c >"${spawning}" 2>"${waited}"
	echo "$?" > "${c_exitfile}"

	setup_check "test_fork_func -c spawning"
	cmp "${spawning}" "${scriptdir}/fork_func/check_order_spawning.good"
	echo "$?" > "${c_exitfile}"

	setup_check "test_fork_func -c waited"
	cmp "${waited}" "${scriptdir}/fork_func/check_order_waited.good"
	echo "$?" > "${c_exitfile}"
}
