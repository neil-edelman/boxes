#!/bin/sh
set -e
cd `dirname $0`
for PROJ in *; do if [[ -d "$PROJ" && ! -L "$PROJ" ]]; then
	# fixme: search subdirs
	[ -d $PROJ/.git/ ] && echo Recusing into $PROJ... \
		&& cd $PROJ && git status && cd ..
fi done
