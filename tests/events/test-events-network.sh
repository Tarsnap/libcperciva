#!/bin/sh

ADDR="[127.0.0.1]:8005"

# Run tests
for i in 1 2 ; do
	printf "%s\n" "==== test $i ===="
	./network_server/test_events_network_server "${ADDR}" &
	pid=$!

	# Wait for the server to be ready
	sleep 1

	# We expect this to work a few times, then fail because
	# the server shut itself down.  Ignore any errors here,
	# because verification will be done by the logfile.
	./network_client/test_events_network_client "${ADDR}" "${pid}" "$i" \
	    2> /dev/null
done
