#!/bin/sh

WARM_UP=10

# Keep this as a global variable to avoid it not counting as a position
# variable in sh.
LOGFILE=$1

# Do more testing if we're running it manually.
if [ -n "${LOGFILE}" ]; then
	MIN_OUT_OF=100
else
	MIN_OUT_OF=10
fi

run() {
	SETS=$1
	REPS=$2
	MPOOL=$3
	SUFFIX_THIS=$4

	# "Warm up"; don't record this data
	for j in `seq ${WARM_UP}`; do
		./test_mpool ${SETS} ${REPS} ${MPOOL} > /dev/null
	done

	arr=""
	# Get raw data
	for i in `seq ${MIN_OUT_OF}`; do
		usec=$( ./test_mpool ${SETS} ${REPS} ${MPOOL} )
		arr="${arr} ${usec}"
	done

	if [ -n "${LOGFILE}" ]; then
		filename="${LOGFILE}-${SUFFIX_THIS}.txt"
		rm -f ${filename}
		for a in ${arr}; do
			echo ${a} >> ${filename}
		done
	fi

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
	SUFFIX=$4

	run $SETS $REPS 0 "${SUFFIX}-malloc"
	malloc=${val}
	run $SETS $REPS 1 "${SUFFIX}-mpool"
	mpool=${val}

	ratio=$( echo "scale=2;${malloc}/${mpool}" | bc)
	ratio_percent=$( echo "100*${malloc}/${mpool}" | bc)

	printf "${SETS}\t${REPS}\t${ratio}\n"
	if [ $(( ${ratio_percent} )) -lt ${PERCENT_CUTOFF} ]; then
		printf "mpool is less than ${PERCENT_CUTOFF}%% of malloc speed\n"
		exit 1
	fi
}


# Test with full benefit from malloc pool (the binary begins with an
# initial pool of 100).
cmp_methods 10000 100 200 "full"

# mpool is not much slower than malloc in this range
# (it's generally faster, but we can't reliably test for that).
cmp_methods 1000 1000 90 "partial"

# mpool is not much slower than malloc even when there's no benefit
# from the pool.
cmp_methods 1 1000000 90 "none"

# Test again with valgrind (if enabled).
if [ -n "${c_valgrind_cmd}" ]; then
	${c_valgrind_cmd} ./test_mpool 100 10000 1 > /dev/null
fi

# Regenerate "all timing data" for gnuplot stats
if [ -n "${LOGFILE}" ]; then
	filename=${LOGFILE}.txt
	rm -f ${filename}
	touch ${filename}
	printf "#full-malloc\tfull-mpool" >> ${filename}
	printf "\tpartial-malloc\tfull-mpool" >> ${filename}
	printf "\tnone-malloc\tnone-mpool" >> ${filename}
	printf "\n" >> ${filename}
	paste	${LOGFILE}-full-malloc.txt	\
		${LOGFILE}-full-mpool.txt	\
		${LOGFILE}-partial-malloc.txt	\
		${LOGFILE}-partial-mpool.txt	\
		${LOGFILE}-none-malloc.txt	\
		${LOGFILE}-none-mpool.txt	\
		>> ${filename}
	rm -f ${LOGFILE}-*.txt
fi
