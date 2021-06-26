#!/bin/bash

# Pushes all git repositories immediately under the directory.
# Not very safe.

cd "$(dirname "$0")" || exit
git pull || true
for PROJ in *; do if [[ -d "$PROJ" && -d "$PROJ/.git" && ! -L "$PROJ" ]]; then
	(
	cd "$PROJ" || exit
	echo "*** Pulling $PROJ... ***"
	git pull || true
	)
fi done
