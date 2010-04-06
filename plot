#!/bin/bash
echo "
	set terminal png
	set output \"$1.png\"
	set xlabel \"Request\"
	set ylabel \"ms\"
	plot \"$1\" using 10 with lines title \"Response time\"" | gnuplot
open $1.png
