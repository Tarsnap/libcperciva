#!/bin/sh

### Constants
c_valgrind_min=1
passdir=${scriptdir}/readpass_file


file_success() {
	filename=$1
	expect_pass=$2

	setup_check_variables
	output="${s_basename}-${s_count}.stdout"
	${c_valgrind_cmd}			\
	    ./test_readpass_file ${filename}	\
	    1> ${output}
	echo "$?" > ${c_exitfile}

	setup_check_variables
	grep -q "${expect_pass}" "${output}"
	echo "$?" > ${c_exitfile}
}

file_fail() {
	filename=$1
	expect_fail=$2

	setup_check_variables
	err_output="${s_basename}-${s_count}.stderr"
	${c_valgrind_cmd}			\
	    ./test_readpass_file ${filename}	\
	    2> ${err_output}
	expected_exitcode 1 "$?" > ${c_exitfile}

	setup_check_variables
	grep -q "${expect_fail}" "${err_output}"
	echo "$?" > ${c_exitfile}
}

### Actual command
scenario_cmd() {
	cd ${scriptdir}/readpass_file

	# Easy password files
	file_success good-noeol.pass "hunter2"
	file_success good-eol.pass "hunter2"
	file_success good-eol-r.pass "hunter2"

	# Don't strip tabs or spaces
	file_success good-spaces.pass " hunter2 "
	file_success good-tabs.pass "	hunter2	"

	# Blank (but acceptable) files
	file_success good-empty.pass ""
	file_success good-only-newline.pass ""

	# These should fail
	file_fail this-file-does-not-exist	\
	    "test_readpass_file: fopen(this-file-does-not-exist)"
	file_fail bad-initial-newline.pass	\
	    "test_readpass_file: line too long, or more than 1 line in"
	file_fail bad-extra-newline.pass	\
	    "test_readpass_file: line too long, or more than 1 line in"
	file_fail bad-extra-material.pass	\
	    "test_readpass_file: line too long, or more than 1 line in"
}
