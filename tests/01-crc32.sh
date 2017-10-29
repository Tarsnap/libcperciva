#!/bin/sh

### Constants
test_output="${s_basename}-stdout.txt"

### Actual command
scenario_cmd() {
	setup_check_variables
	cd ${scriptdir}/crc32 && \
	    ./test_crc32 1> ${test_output}
	echo $? > ${c_exitfile}
}
