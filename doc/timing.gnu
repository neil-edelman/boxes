set style line 1 lt 5 lw 2 lc rgb '#0072bd'
set style line 2 lt 5 lw 2 lc rgb '#ff0000'
set style line 3 lt 5 lw 2 lc rgb '#00ac33'
set style line 4 lt 5 lw 2 lc rgb '#19d3f5'
set term postscript eps enhanced color
set output "graph/timing.eps"
set grid
set xlabel "elements"
set ylabel "time per element, t (ns)"
set yrange [0:2000]
set log x
plot \
"graph/std.tsv" using 1:($2/$1*1000):($3/$1*1000) with errorlines title "std" ls 1, \
"graph/o3.tsv" using 1:($2/$1*1000):($3/$1*1000) with errorlines title "o3" ls 2, \
"graph/o4.tsv" using 1:($2/$1*1000):($3/$1*1000) with errorlines title "o4" ls 3, \
"graph/o128.tsv" using 1:($2/$1*1000):($3/$1*1000) with errorlines title "o128" ls 4, \
"graph/o257.tsv" using 1:($2/$1*1000):($3/$1*1000) with errorlines title "o257" ls 5, \
"graph/o2049.tsv" using 1:($2/$1*1000):($3/$1*1000) with errorlines title "o2049" ls 6
