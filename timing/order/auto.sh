#!/bin/sh

make clean
make release
bin/timing $1 10000 >> timing4.tsv
bin/timing $1 100000 >> timing5.tsv
bin/timing $1 1000000 >> timing6.tsv
