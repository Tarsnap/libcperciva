#!/bin/sh -e

./test_getopt
./test_getopt -
./test_getopt -b
./test_getopt --bar
./test_getopt -f bar
./test_getopt --foo bar
./test_getopt --foo=bar
./test_getopt -bb
./test_getopt -bbf bar
./test_getopt -bbfbar
./test_getopt foo bar baz
./test_getopt -b --foo=bar baz
${c_valgrind_cmd} ./test_getopt -b -- --foo bar baz

if ./test_getopt -a; then
	false;
fi
if ./test_getopt -f; then
	false;
fi
if ./test_getopt --foo; then
	false;
fi
if ./test_getopt --bar=foo; then
	false;
fi

echo "done" 1>&2
