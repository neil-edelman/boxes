#!/bin/bash

# Automatically compiles and tests all of the git projects.
# Assuming projects have a foo/bin/foo test is the default and
# returns 0 for success.

set -e
C="\033[1;36m\033[1m"
N="\033[0m"
cd "$(dirname "$0")" || exit
# -x "$PROJ/bin/$PROJ" No, we should make it.
for PROJ in *; do if [[ -d "$PROJ" && -d "$PROJ/.git" && ! -L "$PROJ" ]]; then
	(
	cd "$PROJ"
	echo -e "${C}*** Making $PROJ ***${N}"
	make
	echo -e "${C}*** Testing $PROJ/bin/$PROJ ***${N}"
	"bin/$PROJ";
	)
	fi
done
