#!/bin/sh

# Allow overriding the default compilers and flags to test.
CCS="${CCS:-c99 cc clang gcc}"
FLAGS="${FLAGS:--O0 -O1 -O2}"

# Working variables.
bin=test_getopt_longjmp
failed=""

for CC in ${CCS}
do
	# Reset posix-flags.sh for each compiler.
	rm -f ../../posix-flags.sh

	for O in ${FLAGS}
	do
		# Check each compiler-flag combination.
		thisbin=$CC$O
		make clean && make CC=$CC CFLAGS=$O
		if ./${bin} ; then
			# Don't keep a working binary.
			rm ${bin}
		else
			# Keep track of failures.
			failed="$failed $thisbin"
			mv $bin $thisbin
		fi
	done
done

# Don't leave objects around which might not be compiled with the
# regular compiler; that can cause problems for the generic `make test`.
make clean

# Notify user of which compiler-flag combinations failed.
printf -- "\n\nFailed binaries: %s\n" "${failed}"
