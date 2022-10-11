set style line 1 lt 5 lw 2 lc rgb '#0072bd'
set style line 2 lt 5 lw 2 lc rgb '#ff0000'
set style line 3 lt 5 lw 2 lc rgb '#00ac33'
set style line 4 lt 5 lw 2 lc rgb '#19d3f5'
set term postscript eps enhanced color
set output "timing.eps"
set grid
set xlabel "elements"
set ylabel "time per element, t (ms)"
set yrange [0:]
plot \
"timing-4.tsv" using 2:($3/1000):($4/1000) with errorlines title "Order 4" ls 1, \
"timing-8.tsv" using 2:($3/1000):($4/1000) with errorlines title "Order 8" ls 2, \
"timing-16.tsv" using 2:($3/1000):($4/1000) with errorlines title "Order 16" ls 1, \
"timing-32.tsv" using 2:($3/1000):($4/1000) with errorlines title "Order 32" ls 2, \
"timing-64.tsv" using 2:($3/1000):($4/1000) with errorlines title "Order 64" ls 1, \
"timing-128.tsv" using 2:($3/1000):($4/1000) with errorlines title "Order 128" ls 2, \
"timing-256.tsv" using 2:($3/1000):($4/1000) with errorlines title "Order 256" ls 1
