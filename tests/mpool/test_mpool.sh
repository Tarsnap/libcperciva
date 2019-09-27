#!/bin/sh

LOGFILE=$1

WARM_UP=5

# Do more testing if we're running it manually.  Using (4n+3) data
# points makes it trivial to split the data into quadrants.
if [ -n "${LOGFILE}" ]; then
	N=30
	REPEATS=$(( 4*N + 3 ))
else
	N=2
	REPEATS=$(( 4*N + 3 ))
fi

# Generate loop variables
make_count() {
	END=$1
	N=1
	while [ $N -le ${END} ]; do
		echo $N
		N=$((N + 1))
	done
}
WARMUP_REPS=$( make_count ${WARM_UP} )
BINARY_REPS=$( make_count ${REPEATS} )

run() {
	SETS=$1
	REPS=$2
	MPOOL=$3
	SUFFIX_THIS=$4

	# "Warm up"; don't record this data
	for j in ${WARMUP_REPS}; do
		./test_mpool ${SETS} ${REPS} ${MPOOL} > /dev/null
	done

	arr=""
	# Get raw data
	for i in ${BINARY_REPS}; do
		usec=$( ./test_mpool ${SETS} ${REPS} ${MPOOL} )
		arr="${arr} ${usec}"
	done

	# Save data to log (if applicable)
	if [ -n "${LOGFILE}" ]; then
		filename="${LOGFILE}-${SUFFIX_THIS}.txt"
		rm -f ${filename}
		num=$(( SETS * REPS ))
		for a in ${arr}; do
			amortized=$( echo "scale=8;${a} / ${num};" | bc)
			echo ${amortized} >> ${filename}
		done
	fi

	## Sort array, find minimum
	sorted_arr=$( echo ${arr} | tr " " "\n" | sort -n | tr "\n" " " )
	lowest=$( echo ${sorted_arr} | cut -d ' ' -f 1 )
	val=${lowest}

	return 0
}

cmp_methods () {
	SETS=$1
	REPS=$2
	PERCENT_CUTOFF=$3
	SUFFIX=$4

	run $SETS $REPS 0 "${SUFFIX}-malloc"
	malloc=${val}
	run $SETS $REPS 1 "${SUFFIX}-mpool"
	mpool=${val}

	ratio=$( echo "scale=2;${malloc}/${mpool}" | bc)
	ratio_percent=$( echo "100*${malloc}/${mpool}" | bc)

	printf "${SETS}\t${REPS}\t${ratio}\n"
	if [ $(( ${ratio_percent} )) -lt ${PERCENT_CUTOFF} ]; then
		# Don't include \n in this message.
		printf "mpool was only ${ratio_percent}%%; wanted " 1>&2
		printf "${PERCENT_CUTOFF}%% of malloc speed... " 1>&2
		exit 1
	fi
}


printf "sets\treps\tratio\n"

# Test with full benefit from malloc pool (the binary begins with an
# initial pool of 100).
cmp_methods 10000 100 200 "full"

# mpool is still considerably faster than malloc in this range.
cmp_methods 1000 1000 200 "partial"

# mpool is not much slower than malloc even when there's no benefit
# from the pool.
cmp_methods 1 1000000 75 "none"

# Test again with valgrind (if enabled).
if [ -n "${c_valgrind_cmd}" ]; then
	${c_valgrind_cmd} ./test_mpool 100 10000 1 > /dev/null
fi
