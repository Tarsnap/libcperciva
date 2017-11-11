# Run as:
#     gnuplot -c mpool-timing.gnuplot FILENAME [Y_MAX]

set term pngcairo size 800,600
set output ARG1.".png"

set multiplot layout 1,3 title "(expected) benefits of mpool"

set xlabel "iterations"
set ylabel "amortized pico-seconds"
unset xtics

stats ARG1 matrix nooutput
set yrange [0:1.01*STATS_max]
if (strlen(ARG2) > 0) {
	set yrange [0:ARG2]
}

set key box opaque
set key at screen 1, screen 1

mids="full partial none"

do for [i=1:words(mids)]{
	mid = word(mids, i)
	set title mid
	malloc_i = 2*i - 1
	mpool_i = 2*i
	plot	ARG1 u 0:malloc_i title "malloc",	\
		ARG1 u 0:mpool_i title "mpool"
	unset key
	unset ylabel
}

#pause -1
