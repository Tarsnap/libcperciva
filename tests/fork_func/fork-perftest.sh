#!/bin/sh

NUM_REPS=31
LIST="0 1 30 90"

for sec in ${LIST}; do
	./test_fork_func -t -d "${sec}" -n "${NUM_REPS}" > "perftest-${sec}.txt"
done

for sec in ${LIST}; do
	for n in 0 1 2; do
		grep "^${n}" "perftest-${sec}.txt" > "perftest-${sec}-${n}.txt"
	done
done
