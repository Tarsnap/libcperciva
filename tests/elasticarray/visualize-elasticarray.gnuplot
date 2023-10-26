set style data boxplot

set key top left
set xrange [0:9]

set title "elasticarray perftest"
set ylabel "time (nanoseconds)"

plot	'old.txt' u (1):1 lc 1 t "append old",	\
	'new.txt' u (2):1 lc 2 t "append new",	\
	'old.txt' u (4):2 lc 1 t "shrink old",	\
	'new.txt' u (5):2 lc 2 t "shrink new",	\
	'old.txt' u (7):3 lc 1 t "both old",	\
	'new.txt' u (8):3 lc 2 t "both new"

pause -1
