set style line 1 lt 5 lw 2 lc rgb '#43fd65'
set style line 2 lt 5 lw 2 lc rgb '#0072bd'
set style line 3 lt 5 lw 2 lc rgb '#ff0000'
set style line 4 lt 5 lw 2 lc rgb '#00ac33'
set style line 5 lt 5 lw 2 lc rgb '#19d3f5'
set term postscript eps enhanced color
set output "compare-look-before.eps"
set grid
set xlabel "keys"
set ylabel "amortized time per key, t ({/Symbol m}s)"
set yrange [0:0.5]
set xrange [1:8388608]
set log x
plot \
"graph-before/table-look.tsv" using 1:($2/$1):($3/$1) with errorlines title "hash table look" ls 2, \
"graph-before/tree-look.tsv" using 1:($2/$1):($3/$1) with errorlines title "tree look" ls 4, \
"graph-before/trie-look.tsv" using 1:($2/$1):($3/$1) with errorlines title "trie look" ls 5
