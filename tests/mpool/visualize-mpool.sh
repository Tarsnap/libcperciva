#!/bin/sh

LOGFILE=$1
YMAX=$2

# Check args
if [ ! -n "${LOGFILE}" ]; then
	echo "Usage: $0 LOGFILE_BASENAME [YMAX]"
	exit 1
fi

# Set up filenames
filename=${LOGFILE}-all.txt
filename_png=${LOGFILE}-all.png
rm -f ${filename}

# Combine all timing data for gnuplot stats
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

# Generate plot
gnuplot -c visualize-mpool.gnuplot ${filename} ${filename_png} ${YMAX}
