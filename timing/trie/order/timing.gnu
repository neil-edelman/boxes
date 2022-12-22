set style line 1 lt 5 lw 2 lc rgb (79,166,220)
set style line 2 lt 5 lw 2 lc rgb '#0072bd'
set style line 3 lt 5 lw 2 lc rgb '#ff0000'
set style line 4 lt 5 lw 2 lc rgb '#00ac33'
set style line 5 lt 5 lw 2 lc rgb '#19d3f5'
set term postscript eps enhanced color
set output "timing.eps"
set grid
set xlabel "order"
set ylabel "time, t (ms)"
set yrange [0:]
plot \
"timing4.tsv" using 1:($3/1000):($4/1000) with errorlines title "10000 keys" ls 2,\
"timing5.tsv" using 1:($3/1000):($4/1000) with errorlines title "100000 keys" ls 4,\
"timing6.tsv" using 1:($3/1000):($4/1000) with errorlines title "1000000 keys" ls 5
