#!/bin/bash

# `cdoc` is used to generate .md files of git repositories
# preferably PROJ/src/PROJ.c or alternately PROJ.h.
# Very specific directory structure and `cdoc` is assumed to exist.
# Overwrites readme.md in projects.

set -e
C="\033[1;36m\033[1m"
N="\033[0m"
cd "$(dirname "$0")" || exit
for PROJ in *; do if [[ -d "$PROJ" && -d "$PROJ/.git" && ! -L "$PROJ" ]]; then
	(
	cd "$PROJ" || exit
	if [[ -r "src/$PROJ.c" ]]; then
		echo -e "${C}*** Updating $PROJ/readme with $PROJ/src/$PROJ.c ***${N}";
		cdoc "src/$PROJ.c" -o readme.md;
		cdoc "src/$PROJ.c" -o readme.html;
	elif [[ -r "src/$PROJ.h" ]]; then
		echo -e "${C}*** Updating $PROJ/readme with $PROJ/src/$PROJ.h ***${N}";
		cdoc "src/$PROJ.h" -o readme.md;
		cdoc "src/$PROJ.h" -o readme.html;
	fi
	)
fi done
