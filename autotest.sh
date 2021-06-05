#!/bin/bash

# Automatically compiles and tests all of the git projects.
# Assuming projects have a foo/bin/foo test is the default and
# returns 0 for success.

set -e
cd "$(dirname "$0")" || exit
# -x "$PROJ/bin/$PROJ" No, we should make it.
for PROJ in *; do if [[ -d "$PROJ" && -d "$PROJ/.git" && ! -L "$PROJ" ]]; then
	(
	cd "$PROJ"
	make
	echo "*** Testing $PROJ/bin/$PROJ ***"
	"bin/$PROJ";
	)
	fi
done
