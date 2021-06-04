#!/bin/sh
set -e
cd `dirname $0`
for PROJ in *; do
	# fixme: search subdirs
	if [[ -d "$PROJ" && ! -L "$PROJ" && -x "$PROJ/bin/$PROJ" ]]; then
		cd $PROJ
		# also do cdoc
		make
		echo Initiating test $PROJ/bin/$PROJ;
		bin/$PROJ;
		cd ..
	fi
done
