#!/bin/sh

pidfile="test_events_pidfile.txt"

# Run normal tests
./test_events > test-events.log

# Compare with good values
cmp -s test-events.good test-events.log

# Run a loop without any events
rm -f "${pidfile}"
./test_events "${pidfile}" > test-empty-events.log &
while [ ! -e "${pidfile}" ] ; do
	sleep 1
done
pid=$( cat "${pidfile}" )
kill -s USR1 ${pid}

# Compare with good values
cmp -s test-empty-events.good test-empty-events.log
