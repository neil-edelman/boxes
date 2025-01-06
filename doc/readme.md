Documentation for the individual boxes is contained in subfolders.

## Ideas

* Maybe deque will be useful? a linked list of nodes of increasing size, starts at 64, starts at 1/3 the way in. `DEQUE_HEAP_ONLY`. This would be great for `cdoc`—cringe on `index_array`.
* Maybe more error reporting goes in `box.h`? I'm hesitant because people have their own way to do it, but a static array of chars for `to_string` is already there.
* Rethink `compare.h` and `iterate.h`. These are the first things I did, and it's problematic. More granularity.
* 1-based height on tree and trie.
* Have trunk (root), contiguous bough, branch, leaf, same in `tree` and `trie`. Turns out, they are very similar and should work the same.
* Split `cursor` and `view` in `tree` and `trie`—they are different.
* Change the font on `html` mode of `cdoc`?
* Put prototypes-support into `cdoc`. Include ignoring __*. Update to `C99`.
* `cdoc` gives errors about not finding stuff, even though it works. Give a clear direction of where it will search.
* Get `cdoc` to recognize `.svg`.
* `cdoc` should be named `boxdoc` …`cboxdoc`?

## Nomenclature

> iterator cursor look range subset span view (means similar in sql): unbounded laden full entire, laden occupied [exists], distance size count, get acquire look first [begin] front, last end back, shift pop_front (sounds like modifying data) advance (doesn't fit with front) (I kind of like \_front) ditch_ dump_ can_ oust_ expel_ drop_front cut_ drain_ eject_ shed_, pop pop_back reverse retract regress recede withdraw retire recede… random?, delete, insert
