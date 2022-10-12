set style line 1 lt 5 lw 2 lc rgb '#0072bd'
set style line 2 lt 5 lw 2 lc rgb '#ff0000'
set style line 3 lt 5 lw 2 lc rgb '#00ac33'
set style line 4 lt 5 lw 2 lc rgb '#19d3f5'
set term postscript eps enhanced color
set output "timing.eps"
set grid
set xlabel "elements"
set ylabel "time per element, t (ns)"
set yrange [0:]
set log x
plot \
"table.tsv" using 1:($2/$1*1000):($3/$1*1000) with errorlines title "table" ls 1, \
"tree.tsv" using 1:($2/$1*1000):($3/$1*1000) with errorlines title "tree" ls 2, \
"trie.tsv" using 1:($2/$1*1000):($3/$1*1000) with errorlines title "trie" ls 3
