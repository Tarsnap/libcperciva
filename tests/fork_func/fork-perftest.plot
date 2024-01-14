# Run with:
#     gnuplot -c fork-perftest.plot

list = "0 1 30 90"
YMIN = -0.05
YMAX = 1.0

set style data boxplot
set ylabel "extra delay ms"
set xlabel "type of operation"
set key left

set xtics ("no multi" 0, "pthread" 1, "fork\\\_func" 2)
set xrange [-1:3]
set yrange [YMIN:YMAX]

# Sanity check for YMAX.
do for [i in list] {
	stats "perftest-".i.".txt" using 2 prefix "S" nooutput
	if (S_max > YMAX) {
		print "Y max too small: ", S_max, YMAX
		exit 1
	}
}

# Do the actual plotting.
do for [i in list] {
	set title "Extra delay when waiting ".i." ms"

	plot "perftest-".i.'-0.txt' title "no multi"
	replot "perftest-".i.'-1.txt' title "pthread"
	replot "perftest-".i.'-2.txt' title "fork\\\_func"

	pause -1
}
