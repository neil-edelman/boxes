set term postscript eps enhanced color
set style line 1 lt 5 lw 2 lc rgb '#0072bd'
set output "graph/order.eps"
set grid
set xlabel "order"
set ylabel "time to add 100000 elements, t (ns)"
set xrange [0:500]
set yrange [15:25]
#set xtics 50
plot "graph/order.tsv" using 1:($2/1000):($3/1000) with errorlines title "order" ls 1
