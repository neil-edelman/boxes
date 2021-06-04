#!/bin/sh
# fixme: this is not the way
[ -z $1 ] && echo "Commit message." && exit
cd `dirname $0`
for PROJ in *; do if [[ -d "$PROJ" && ! -L "$PROJ" ]]; then
	[ -d $PROJ/.git/ ] && echo Recusing into $PROJ... \
		&& cd $PROJ && ((git commit -am \""$1"\") || [ -d .git/ ] ) && cd ..
fi done
