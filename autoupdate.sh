#!/bin/sh
set -e
cd `dirname $0`
for PROJ in *; do if [[ -d "$PROJ" && ! -L "$PROJ" ]]; then
	[ -r $PROJ/update.txt ] && echo $PROJ && while read -r LINE; do
		[ -r $PROJ/src/$PROJ.c ] && echo "$PROJ/src/$PROJ.c => $LINE" \
			&& cp $PROJ/src/$PROJ.c $LINE;
		[ -r $PROJ/src/$PROJ.h ] && echo "$PROJ/src/$PROJ.h => $LINE" \
			&& cp $PROJ/src/$PROJ.h $LINE;
	done < $PROJ/update.txt
fi done

# links screw up on Mac
#[ -r update.txt ] && while read -r LINE; do ln -fv src/Array.h $LINE; done < update.txt
