#!/bin/bash

# Pushes all git repositories immediately under the directory.
# Not very safe.

cd "$(dirname "$0")" || exit
echo "*** Pushing . ... ***"
git push || true
for PROJ in *; do if [[ -d "$PROJ" && -d "$PROJ/.git" && ! -L "$PROJ" ]]; then
	(
	cd "$PROJ" || exit
	echo "*** Pushing $PROJ... ***"
	git push || true
	)
fi done
