#!/bin/bash

# Downloads all the boxes. One can download any
# number of them, but I found this to be frequent
# enough to warrant it's own script.

set -e
cd "$(dirname "$0")" || exit
[[ ! -e "array" ]] && git clone https://github.com/neil-edelman/array
[[ ! -e "bmp" ]] && git clone https://github.com/neil-edelman/bmp
[[ ! -e "heap" ]] && git clone https://github.com/neil-edelman/heap
[[ ! -e "list" ]] && git clone https://github.com/neil-edelman/list
[[ ! -e "orcish" ]] && git clone https://github.com/neil-edelman/orcish
[[ ! -e "pool" ]] && git clone https://github.com/neil-edelman/pool
[[ ! -e "table" ]] && git clone https://github.com/neil-edelman/table
[[ ! -e "trie" ]] && git clone https://github.com/neil-edelman/trie
