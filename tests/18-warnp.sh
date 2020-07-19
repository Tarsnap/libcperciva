#!/bin/sh

### Constants
c_valgrind_min=1
test_output="${s_basename}-stderr.txt"

# Check that the output matches what's expected.  We can't use a "good output"
# file because strerror() returns a locale-dependent error message.
check_output() {
	filename=$1
	nonce=$2

	# Check that the output contains the expected nonce.
	test "$(grep -c "nonce=${nonce}" "${filename}")" -eq "1"

	# We should have 2 lines with "warnp errno".
	test "$(grep -c "warnp errno" "${filename}")" -eq "2"

	# We should have 7 lines in total.
	test "$(wc -l < "${filename}")" -eq "7"

	# We should have 2 lines without a colon.
	test "$(grep -c -v "test_warnp:" "${filename}")" -eq "2"

	# We should have 5 lines with a colon.
	test "$(grep -c "test_warnp:" "${filename}")" -eq "5"

	# We should have 1 line with "~_________" from the long message.
	test "$(grep -c "~_________" "${filename}")" -eq "1"
}

### Actual command
scenario_cmd() {
	cd ${scriptdir}/warnp || exit

	# Run binary.
	nonce="normal"
	setup_check_variables "test_warnp"
	${c_valgrind_cmd}			\
	    ./test_warnp "${nonce}" 2> ${test_output}
	echo "$?" > ${c_exitfile}

	# Check console output.
	setup_check_variables "test_warnp output console"
	$(set -e ; check_output "${test_output}" "${nonce}")
	echo "$?" > ${c_exitfile}
}
