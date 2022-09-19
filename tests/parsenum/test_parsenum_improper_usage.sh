#!/bin/sh

# Temporary output
log=abort.log
rcfile=rc.txt

# Check test_parsenum case
run_test() {
	num=$1
	reason=$2

	# Run the command with no command-line output.  The binary should
	# produce an abort(), which sends a SIGABRT to the calling process.
	# Upon receiving this signal, different shells react in different
	# ways.  In order to avoid polluting the output of `make test`, we
	# redirect any such message to /dev/null.  In order to retain the
	# exit code of ./test_parsenum, we write the value to a file within
	# the smallest subshell.  We found that redirecting from both () and
	# $(), and writing the exit code to a file, was necessary to cover
	# Linux, FreeBSD, MacOS X, and Solaris.
	(
	    $(./test_parsenum $1 2>${log}; echo "$?" > "${rcfile}" ) 2>/dev/null
	) 2>/dev/null
	rc=$( cat "${rcfile}" )

	# Check the exit code.  Many shells will set the exit code to 134
	# (being 128 + SIGABRT), but some do not.  POSIX merely specifies that
	# the value shall be greater than 128.
	if [ ! "$rc" -gt 128 ]; then
		printf "Return value not higher than 128\n"
		exit 1
	fi

	# Check the reason for this abort.
	if ! grep -q "${reason}" ${log} ; then
		printf "Correct reason not found in %s\n" "${log}"
		exit 1
	fi
}

# Run tests.  Test 4 adds min/max bounds to the PARSENUM_EX call in test 3.
run_test 1 "PARSENUM applied to signed integer without specified bounds"
run_test 2 "PARSENUM_EX applied to signed integer without specified bounds"
run_test 3 "PARSENUM_EX applied to float with base != 0"
run_test 4 "PARSENUM_EX applied to float with base != 0"

# Clean up
rm $log
rm $rcfile
