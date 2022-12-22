set style line 1 lt 5 lw 2 lc rgb '#43fd65'
set style line 2 lt 5 lw 2 lc rgb '#0072bd'
set style line 3 lt 5 lw 2 lc rgb '#ff0000'
set style line 4 lt 5 lw 2 lc rgb '#00ac33'
set style line 5 lt 5 lw 2 lc rgb '#19d3f5'
set term postscript eps enhanced color
set output "compare-add-before.eps"
set grid
set xlabel "keys"
set ylabel "amortized time per key, t ({/Symbol m}s)"
set yrange [0:3]
set xrange [1:8388608]
set log x
plot \
"graph-before/table-add.tsv" using 1:($2/$1):($3/$1) with errorlines title "hash table add" ls 2, \
"graph-before/tree-add.tsv" using 1:($2/$1):($3/$1) with errorlines title "tree add" ls 4, \
"graph-before/trie-add.tsv" using 1:($2/$1):($3/$1) with errorlines title "trie add" ls 5
