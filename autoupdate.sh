#!/bin/bash

# Any matching project names and source files, _eg_, foo/src/foo.h (or .c),
# and any files in `traits` will be considered one-source-of-truth and will
# be updated in all the other project's src/ and test/.
# Fairly dangerous!
# Links would be great for permanent solutions to this problem,
# but they are unsupported in Xcode and in general on Mac GUI.

set -e
cd "$(dirname "$0")" || exit
for PROJ in *; do if [[ -d "$PROJ" && -d "$PROJ/.git" && ! -L "$PROJ" ]]; then
	for EXT in c h; do if [[ -r "$PROJ/src/$PROJ.$EXT" ]]; then
		for OTHER in *; do if [[ -d "$OTHER" && -d "$OTHER"/.git/ \
			&& ! -L "$OTHER" && "$OTHER" != "$PROJ" ]]; then
			for SUB in src test; do
				[ -r "$OTHER/$SUB/$PROJ.$EXT" ] \
				&& echo "Updating Truth $PROJ/src/$PROJ.$EXT -> $OTHER/$SUB/$PROJ.$EXT." \
				&& cp "$PROJ/src/$PROJ.$EXT" "$OTHER/$SUB/$PROJ.$EXT"
			done
		fi done
	fi done
fi done
for TRAITDIR in traits/*; do
	TRAIT=$(basename "$TRAITDIR")
	for PROJ in *; do if [[ -d "$PROJ" && -d "$PROJ/.git" && ! -L "$PROJ" ]]; then
		for SUB in src test; do [ -r "$PROJ/$SUB/$TRAIT" ] \
			&& echo "Updating Truth $TRAITDIR -> $PROJ/$SUB/$TRAIT." \
			&& cp "$TRAITDIR" "$PROJ/$SUB/$TRAIT"
		done
	fi done
done
