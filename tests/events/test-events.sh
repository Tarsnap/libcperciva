#!/bin/sh
./test_events > test-events.log

# Compare with good values
cmp -s test-events.good test-events.log
