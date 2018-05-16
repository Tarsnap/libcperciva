#!/bin/sh

### Constants
c_valgrind_min=1
test_stderr="${s_basename}-stderr.txt"

# Keep this in sync with the value in tests/setuidgid/main.c!
EX_NOPERM=77


test_setuidgid_username_groupname() {
	pass_exitcode=$1
	skip_ok=$2
	username_groupname=$3

	setup_check_variables

	# Print info about each check
	printf "=== check ${s_count}" >> ${test_stderr}
	if [ ${pass_exitcode} -ne "0" ]; then
		printf " (should fail)" >> ${test_stderr}
	fi
	if [ ${skip_ok} -ne "0" ]; then
		printf " (can pass despite EPERM)" >> ${test_stderr}
	fi
	printf "\n" >> ${test_stderr}

	# Run command
	cd ${scriptdir}/setuidgid/ && \
	    ${c_valgrind_cmd} ./test_setuidgid "${username_groupname}"	\
	    2>> ${test_stderr}
	cmd_exitcode=$?

	# Evaluate results
	if [ ${cmd_exitcode} -eq "$EX_NOPERM" ]; then
		if [ ${skip_ok} -eq "1" ]; then
			# Check if root
			if [ `id -u` -eq "0" ]; then
				# Fail.
				echo "1"
			else
				# Skip this check, since it relies on us having
				# permission to change the uid and/or gid, and
				# we're not root.
				echo "-1"
			fi
		else
			# Fail
			echo "1"
		fi
	elif [ ${cmd_exitcode} -eq ${pass_exitcode} ]; then
		# We match the expect output.
		echo "0"
	elif [ ${cmd_exitcode} -eq "${valgrind_exit_code}" ]; then
		# Keep any valgrind failure.
		echo "${valgrind_exit_code}"
	else
		# Fail
		echo "1"
	fi > ${c_exitfile}
}

### Actual command
scenario_cmd() {
	# Try as self; should work.
	test_setuidgid_username_groupname 0 0 "$(id -u -n):$(id -g -n)"

	# Try as self with numeric UID and GID; should work.
	test_setuidgid_username_groupname 0 0 "$(id -u):$(id -g)"

	# Try as nobody; accept an EPERM if not root.
	test_setuidgid_username_groupname 0 1 "nobody"
	test_setuidgid_username_groupname 0 1 "nobody:$(id -gn nobody)"

	# Try as nobody with numeric UID and GID; accept EPERM
	test_setuidgid_username_groupname 0 1 "$(id -u nobody)"
	test_setuidgid_username_groupname 0 1 "$(id -u nobody):$(id -g nobody)"

	# These should all fail.  (Hopefully nobody actually has
	# "fake_username" or "fake_groupname" on their systems!)
	test_setuidgid_username_groupname 1 0 "nobody:"
	test_setuidgid_username_groupname 1 0 "fake_username"
	test_setuidgid_username_groupname 1 0 ":fake_groupname"
	test_setuidgid_username_groupname 1 0 "fake_username:fake_groupname"
}
