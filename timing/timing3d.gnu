set term postscript eps enhanced
set output "plot3d.eps"
set xlabel "order"
set ylabel "elements"
set zlabel "time, t (ms)"
set zrange [0:]

set parametric
set hidden3d back offset 0 trianglepattern 3 undefined 1 altdiagonal bentover
set style data lines
set colorbox vertical origin screen 0.9, 0.2 size screen 0.05, 0.6 front  noinvert bdefault

set key noautotitle

set ytics 200000
set ztics 1000
set view 60, 75
set grid z
set colorsequence classic

splot "timing.tsv" using 1:2:($3/1000)
