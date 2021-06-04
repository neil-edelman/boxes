#!/bin/sh
set -e
cd `dirname $0`
for PROJ in *; do
	# fixme: search subdirs
	if [[ -d "$PROJ" && ! -L "$PROJ" ]]; then
		cd $PROJ
		# echo "In $PROJ."
		if [[ -r src/$PROJ.c ]]; then
			echo "Updating $PROJ/readme with $PROJ/src/$PROJ.c";
			cdoc src/$PROJ.c -o readme.md;
			cdoc src/$PROJ.c -o readme.html;
		elif [[ -r src/$PROJ.h ]]; then
			echo "Updating $PROJ/readme with $PROJ/src/$PROJ.h";
			cdoc src/$PROJ.h -o readme.md;
			cdoc src/$PROJ.h -o readme.html;
		fi
		cd ..
	fi
done
