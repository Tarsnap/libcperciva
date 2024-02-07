# General setup
set term pngcairo size 800,600
set output ARG2

# Titles, axes, and key
set title "(expected) benefits of mpool"
set ylabel "amortized micro-seconds"

if (strlen(ARG3) > 0) {
	set yrange [:ARG3]
}

set key box opaque
set key at screen 1, screen 1

# Box plot
set style fill solid 0.5 border -1
set style boxplot outliers pointtype 7
set style data boxplot
set boxwidth 0.45
set pointsize 0.5

# Ticks
set xtics scale 0
set x2tics scale 0

set x2tics ("full" 1.5, "partial" 3.5, "none" 5.5)
set xtics (	"malloc" 1.2, "mpool" 1.8,	\
		"malloc" 3.2, "mpool" 3.8,	\
		"malloc" 5.2, "mpool" 5.8)

# Plot data
plot	ARG1 u (1.2):1 title "malloc",		\
	ARG1 u (1.8):2 title "mpool",		\
	ARG1 u (3.2):3 title "" lc 1,		\
	ARG1 u (3.8):4 title "" lc 2,		\
	ARG1 u (5.2):5 title "" lc 1,		\
	ARG1 u (5.8):6 title "" lc 2
