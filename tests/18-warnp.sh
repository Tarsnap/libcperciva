#!/bin/sh

### Constants
c_valgrind_min=1
test_output="${s_basename}-stderr.txt"
test_output_multithreaded="${s_basename}-multithreaded-stderr.txt"

# Check that the output matches what's expected.  We can't use a "good output"
# file because strerror() returns a locale-dependent error message.
check_output() {
	filename=$1
	nonce=$2

	# Check that the output contains the expected nonce.
	test "$(grep -c "nonce=${nonce}" "${filename}")" -eq "1"

	# We should have 2 lines with "warnp errno".
	test "$(grep -c "warnp errno" "${filename}")" -eq "2"

	# We should have 8 lines in total.
	test "$(wc -l < "${filename}")" -eq "8"

	# We should have 2 lines without a colon.
	test "$(grep -c -v "test_warnp:" "${filename}")" -eq "2"

	# We should have 6 lines with a colon.
	test "$(grep -c "test_warnp:" "${filename}")" -eq "6"

	# We should have 1 line with "~_________" from the long message.
	test "$(grep -c "~_________" "${filename}")" -eq "1"

	# Check that stderr output was re-established.
	test "$(grep -c "back to stderr" "${filename}")" -eq "1"
}

### Actual command
scenario_cmd() {
	cd "${scriptdir}/warnp" || exit

	# Run binary to check stderr vs. syslog.
	nonce="normal"
	setup_check "test_warnp"
	${c_valgrind_cmd}			\
	    ./test_warnp "${nonce}" 1 2> "${test_output}"
	echo "$?" > "${c_exitfile}"

	# Check console output.
	setup_check "test_warnp output console"
	(set -e ; check_output "${test_output}" "${nonce}")
	echo "$?" > "${c_exitfile}"

	# Run binary to check multithreaded output.
	setup_check "test_warnp multithreaded"
	${c_valgrind_cmd}			\
	    ./test_warnp "${nonce}" 2 2> "${test_output_multithreaded}"
	echo "$?" > "${c_exitfile}"

	# Check multithreaded console output.
	setup_check "test_warnp multithreaded output"
	grep -q -v "test_warnp: " "${test_output_multithreaded}"
	expected_exitcode 1 "$?" > "${c_exitfile}"
}
