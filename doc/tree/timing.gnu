set term postscript eps enhanced color
set output "timing.eps"
set grid
set xlabel "elements"
set ylabel "time per element, t (ns)"
set yrange [0:2000]
set log x
set for [i=1:8] linetype i dashtype i
plot \
"timing-std.tsv" using 1:($2/$1*1000):($3/$1*1000) with errorlines title "std" ls 1, \
"timing-o3.tsv" using 1:($2/$1*1000):($3/$1*1000) with errorlines title "o3" ls 2, \
"timing-o4.tsv" using 1:($2/$1*1000):($3/$1*1000) with errorlines title "o4" ls 3, \
"timing-o128.tsv" using 1:($2/$1*1000):($3/$1*1000) with errorlines title "o128" ls 4, \
"timing-o257.tsv" using 1:($2/$1*1000):($3/$1*1000) with errorlines title "o257" ls 5, \
"timing-o2049.tsv" using 1:($2/$1*1000):($3/$1*1000) with errorlines title "o2049" ls 6
