#!/bin/sh

./test_posix_close /dev/null 0 &
pid=$!

while true ; do
	kill -14 ${pid}
	rc=$?
	if [ ${rc} -ne "0" ]; then
		echo "rc: $rc"
		exit 1
	fi
done

