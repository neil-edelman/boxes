set term postscript eps enhanced
set output "pool_vs_pool.eps"
set xlabel "size"
set ylabel "time us/space bytes"
set multiplot layout 1, 2

plot "pool_vs_pool_time.data" using 1:2 title "new" with lines, \
"pool_vs_pool_time.data" using 1:3 title "old" with lines
plot "pool_vs_pool_space.data" using 1:2 title "new" with lines, \
"pool_vs_pool_space.data" using 1:3 title "old" with lines
