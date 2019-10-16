#!/bin/sh

# Run normal tests
./test_events > test-events.log

# Compare with good values
cmp -s test-events.good test-events.log

# Run a loop without any events
./test_events 1 > test-empty-events.log &
pid=$!
sleep 1
kill -s USR1 ${pid}

# Compare with good values
cmp -s test-empty-events.good test-empty-events.log
