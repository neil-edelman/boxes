set term postscript eps enhanced color
set output "compare-add.eps"
set grid
set monochrome
set xlabel "keys"
set ylabel "amortized time per key, t ({/Symbol m}s)"
set yrange [0:3]
set xrange [1:8388608]
set log x
plot \
"graph/trie-add.tsv" using 1:($2/$1):($3/$1) with errorlines title "trie add", \
"graph/tree-add.tsv" using 1:($2/$1):($3/$1) with errorlines title "tree add", \
"graph/table-add.tsv" using 1:($2/$1):($3/$1) with errorlines title "hash table add"
