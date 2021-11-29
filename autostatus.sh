#!/bin/bash

# This is a non-destructive task (with normal get) that checks the status
# of every project at the top-level directory.

set -e
C="\033[1;36m\033[1m"
N="\033[0m"
cd "$(dirname "$0")" || exit
git status
for PROJ in *; do if [[ -d "$PROJ" && -d "$PROJ/.git" && ! -L "$PROJ" ]]; then
	(
	echo -e "${C}*** Status of $PROJ... ***${N}" && cd "$PROJ" && git status
	)
fi done
