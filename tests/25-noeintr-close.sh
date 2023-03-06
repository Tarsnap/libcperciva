#!/bin/sh

### Constants
c_valgrind_min=1

## open_close(cmd, n, infile):
# Using ${cmd}, repeatedly open ${infile}, read ${n} bytes, then close it.
open_close() {
	cmd=$1
	n=$2
	infile=$3

	# If we're reading from /dev/null, use a different filename for the
	# log file (since slashes would be interpreted as directories).
	infiletxt="${infile}"
	if [ "${infile}" = "/dev/null" ]; then
		infiletxt="devnull"
	fi
	logfile="${s_basename}-stdout-${bintype}-${infiletxt}-${n}.txt"

	setup_check "${cmd} ${infile} ${n}"
	${c_valgrind_cmd}						\
	    ./"${cmd}" "${infile}" "${n}"				\
	    > "${logfile}"
	echo "$?" > "${c_exitfile}"
}

### Actual command
scenario_cmd() {
	for bintype in normal pthread; do
		# Set up command and change to the appropriate directory.
		cmd="test_noeintr_close_${bintype}"
		cd "${scriptdir}/noeintr_close/${bintype}" || exit

		# We can only read 0 bytes from /dev/null.
		open_close "${cmd}" 0 /dev/null

		# We can read 0 or 1 bytes from an arbitary file ("Makefile").
		open_close "${cmd}" 0 Makefile
		open_close "${cmd}" 1 Makefile
	done
}
