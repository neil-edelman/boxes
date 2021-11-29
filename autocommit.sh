#!/bin/bash

# Automatically commits all repositories under the current directory.
# Note that this is probably dangerous and irresponsible. Usually,
# autoupdate && autotest && autocommit "Changed something." && autopush

set -e
C="\033[1;36m\033[1m"
N="\033[0m"
[ -z "$1" ] && echo "Commit message?" && exit
cd "$(dirname "$0")" || exit
for PROJ in *; do if [[ -d "$PROJ" && -d "$PROJ/.git" && ! -L "$PROJ" ]]; then
	(
	echo -e "${C}*** Committing $PROJ... ***${N}"
	cd "$PROJ" || exit
	git commit -am "$1" || true
	)
fi done
git commit -am "$1" || true
