set style line 1 lt 5 lw 2 lc rgb '#0072bd'
set style line 2 lt 5 lw 2 lc rgb '#ff0000'
set style line 3 lt 5 lw 2 lc rgb '#00ac33'
set style line 4 lt 5 lw 2 lc rgb '#19d3f5'
set term postscript eps enhanced color
set output "timing-map-what.eps"
set grid
set xlabel "elements"
set ylabel "time, t (us)"
set yrange [0:]
set log x
plot \
"std-map.tsv" using 1:($2):($3) with errorlines title "std" ls 1, \
"o3-map.tsv" using 1:($2):($3) with errorlines title "o3" ls 2, \
"o128-map.tsv" using 1:($2):($3) with errorlines title "o128" ls 3, \
"o257-map.tsv" using 1:($2):($3) with errorlines title "o257" ls 4, \
"o2049-map.tsv" using 1:($2):($3) with errorlines title "o2049" ls 5
