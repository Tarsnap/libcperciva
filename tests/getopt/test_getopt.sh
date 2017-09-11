#!/bin/sh -e

# The `|| true` ignores the return value of `test_getopt_args.sh`.  This is
# deliberate, since we want to rely on comparing the output with good output
# to determine success or failure.
printf "Testing command-line options parsing... "
/bin/sh -e test_getopt_args.sh 2>test_getopt.log || true
if cmp -s test_getopt.log test_getopt.good; then
	echo "PASSED!"
else
	echo "FAILED!"
	echo "test_getopt.log:"
	cat test_getopt.log
	exit 1
fi
