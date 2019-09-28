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
	SUFFIX=$3

	# "Warm up"; don't record this data
	for j in ${WARMUP_REPS}; do
		./test_mpool ${SETS} ${REPS} 0 > /dev/null
		./test_mpool ${SETS} ${REPS} 1 > /dev/null
	done

	arr_malloc=""
	arr_mpool=""
	# Get raw data
	for i in ${BINARY_REPS}; do
		usec=$( ./test_mpool ${SETS} ${REPS} 0 )
		arr_malloc="${arr_malloc} ${usec}"
		usec=$( ./test_mpool ${SETS} ${REPS} 1 )
		arr_mpool="${arr_mpool} ${usec}"
	done

	# Save data to log (if applicable)
	if [ -n "${LOGFILE}" ]; then
		num=$(( SETS * REPS ))

		# Write malloc times
		filename="${LOGFILE}-${SUFFIX}-malloc.txt"
		rm -f ${filename}
		for a in ${arr_malloc}; do
			amortized=$( echo "scale=8;${a} / ${num};" | bc)
			echo ${amortized} >> ${filename}
		done

		# Write mpool times
		filename="${LOGFILE}-${SUFFIX}-mpool.txt"
		rm -f ${filename}
		for a in ${arr_mpool}; do
			amortized=$( echo "scale=8;${a} / ${num};" | bc)
			echo ${amortized} >> ${filename}
		done
	fi

	## Sort array, find median
	sorted_arr=$( echo ${arr_malloc} | tr " " "\n" | sort -n | tr "\n" " " )
	median=$( echo ${sorted_arr} | cut -d ' ' -f $(( 1 + ${REPEATS}/2 )) )
	val_malloc=${median}

	sorted_arr=$( echo ${arr_mpool} | tr " " "\n" | sort -n | tr "\n" " " )
	median=$( echo ${sorted_arr} | cut -d ' ' -f $(( 1 + ${REPEATS}/2 )) )
	val_mpool=${median}

	return 0
}

cmp_methods () {
	SETS=$1
	REPS=$2
	PERCENT_CUTOFF=$3
	SUFFIX=$4

	run $SETS $REPS "${SUFFIX}"
	malloc=${val_malloc}
	mpool=${val_mpool}

	# Sanity check for 0-duration result
	if [ "${malloc}" -eq "0" ] || [ "${mpool}" -eq "0" ]; then
		# Don't include \n in this message; the test suite
		# will add it.
		printf "Test time is below clock resolution;" 1>&2
		printf " cannot measure" 1>&2
		exit 1
	fi

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
