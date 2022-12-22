set term postscript eps enhanced color
set output "compare-look.eps"
set grid
set monochrome
set xlabel "keys"
set ylabel "amortized time per key, t ({/Symbol m}s)"
set yrange [0:0.5]
set xrange [1:8388608]
set log x
plot \
"graph/trie-look.tsv" using 1:($2/$1):($3/$1) with errorlines title "trie look", \
"graph/tree-look.tsv" using 1:($2/$1):($3/$1) with errorlines title "tree look", \
"graph/table-look.tsv" using 1:($2/$1):($3/$1) with errorlines title "hash table look"
