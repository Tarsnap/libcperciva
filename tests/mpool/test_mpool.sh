#!/bin/sh

MIN_OUT_OF=10
OUT=timing.txt

# Header
echo "sets\treps\tmalloc\tmpool" > ${OUT}

run() {
	SETS=$1
	REPS=$2
	MPOOL=$3

	# "Warm up"; don't record this data
	for j in `seq ${MIN_OUT_OF}`; do
		./test_mpool ${SETS} ${REPS} ${MPOOL} > /dev/null
	done

	arr=""
	# Get raw data
	for i in `seq ${MIN_OUT_OF}`; do
		usec=$( ./test_mpool ${SETS} ${REPS} ${MPOOL} )
		arr="${arr} ${usec}"
	done

	## Sort array, find minimum
	arr=$( echo ${arr} | tr " " "\n" | sort -n | tr "\n" " " )
	lowest=$( echo ${arr} | cut -d ' ' -f 1 )

	val=${lowest}
	return 0
}

cmp_methods () {
	SETS=$1
	REPS=$2
	PERCENT_CUTOFF=$3

	run $SETS $REPS 0
	malloc=${val}
	run $SETS $REPS 1
	mpool=${val}

	ratio=$( echo "scale=2;${malloc}/${mpool}" | bc)
	ratio_percent=$( echo "100*${malloc}/${mpool}" | bc)

	printf "${SETS}\t${REPS}\t${ratio}\n"
	if [ $(( ${ratio_percent} )) -lt ${PERCENT_CUTOFF} ]; then
		printf "mpool is less than ${PERCENT_CUTOFF}%% of malloc speed\n"
		exit 1
	fi
}


printf "sets\treps\tratio\n"

# Test with full benefit from malloc pool (the binary begins with an
# initial pool of 100).
cmp_methods 10000 100 200

# mpool is not much slower than malloc in this range
# (it's generally faster, but we can't reliably test for that).
cmp_methods 1000 1000 90

# mpool is not much slower than malloc even when there's no benefit
# from the pool.
cmp_methods 1 1000000 90

# Test again with valgrind (if enabled).
if [ -n "${c_valgrind_cmd}" ]; then
	${c_valgrind_cmd} ./test_mpool 100 10000 1 > /dev/null
fi
