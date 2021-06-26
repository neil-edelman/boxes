#!/bin/bash

# This is a non-destructive task (with normal get) that checks the status
# of every project at the top-level directory.

set -e
cd "$(dirname "$0")" || exit
git status
for PROJ in *; do if [[ -d "$PROJ" && -d "$PROJ/.git" && ! -L "$PROJ" ]]; then
	(
	echo "*** Status of $PROJ... ***" && cd "$PROJ" && git status
	)
fi done
