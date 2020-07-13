#!/bin/sh

TIME_COMMAND="./test_md5 -t"
CFLAGS_ORIG="${CFLAGS:-}"

for COMPILER in "$@"; do
	for OFLAG in "-O2" "-O3 -march=native"; do
		export CC="$COMPILER"
		if [ -n "${CFLAGS_ORIG}" ]; then
			export CFLAGS="${CFLAGS_ORIG} ${OFLAG}"
		else
			export CFLAGS="${OFLAG}"
		fi
		make clean all >/dev/null 2>/dev/null
		echo "$CC $CFLAGS"
		seq 3 | while read X; do
			${TIME_COMMAND} |
			    grep Time |
			    cut -f 3 -d ' ';
		done |
		    sort -n |
		    head -1
	done;
done | paste -d ",\n" -s -
