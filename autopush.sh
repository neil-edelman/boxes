#!/bin/bash

# Pushes all git repositories immediately under the directory.
# Not very safe.

C="\033[1;36m\033[1m"
N="\033[0m"
cd "$(dirname "$0")" || exit
echo -e "${C}*** Pushing . ... ***${N}"
git push || true
for PROJ in *; do if [[ -d "$PROJ" && -d "$PROJ/.git" && ! -L "$PROJ" ]]; then
	(
	cd "$PROJ" || exit
	echo -e "${C}*** Pushing $PROJ... ***${N}"
	git push || true
	)
fi done
