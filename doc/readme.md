Documentation for the individual boxes is contained in subfolders.

## Issues

On a segmented memory model, `pool` probably won't work reliably,
especially when one compiles with `C90`.

(Especially) `trie` hasn't been tested when `CHAR_BIT ≠ 8`, as it
could be in `C90`.

`C90` requires identifiers to be a minimum of 31 significant initial
characters in an internal identifier or a macro name. If one's
compiler does the very minimum, it probably won't be enough to
differentiate all the private symbols. One may have to edit the
code to abbreviate.

## Fixme

* `graph.h` HAS_GRAPH_H?
* Maybe deque will be useful? a linked list of nodes of increasing size, starts at 64, starts at 1/3 the way in.
* Maybe error reporting goes in `box.h`?
* Rethink `compare.h` and `iterate.h`.
* Get `cdoc` to recognize `.svg`.
* 1-based height on tree and trie.
* Trunk (root), contiguous bough, branch, leaf, same in tree and trie.
* Change the font, eww.

# Interfaces

Ideas:
> iterator cursor look range subset span view (means similar in sql): unbounded laden full entire, laden occupied [exists], distance size count, get acquire look first [begin] front, last end back, shift pop_front (sounds like modifying data) advance (doesn't fit with front) (I kind of like \_front) ditch_ dump_ can_ oust_ expel_ drop_front cut_ drain_ eject_ shed_, pop pop_back reverse retract regress recede withdraw retire recede… random?, delete, insert

## to_string

	view, entire, exists, drop_front, front
	void <type>to_string(const *<type>, char (*)[12]); —supplied by user, [11] is a dud

If turned on by the options, includes `to_string.h` and supplies,

	const char *<T>to_string(const <T> *);

Which calls `<type>to_string` in the order of `entire` `front` and prints in one of 4 static buffers.

## iterate

	view, entire, exists, drop_front, front

On inclusion somehow from `iterate.h`, supplies:

	typedef void (*<T>action_fn)(<T> *);
	typedef int (*<T>predicate_fn)(const <T> *);
	<type> *<T>any(const <T> *, <T>predicate_fn);
	void <T>each(<T> *, <T>action_fn);
	void <T>if_each(<T> *, <T>predicate_fn, <T>action_fn);
	int <T>copy_if(<T> *, const <T> *, <T>predicate_fn); —contiguous
	void <T>keep_if(<T> *, <T>predicate_fn, <T>action_fn); —contiguous
	void <T>trim(<T> *, <T>predicate_fn); —contiguous

I have never used this.

## compare

	view, entire, exists, drop_front, front

On inclusion somehow from `compare.h`, supplies:

	typedef int (*<T>bipredicate_fn)(<type> *, <type> *);
	typedef int (*<T>compare_fn)(const <type> *, const <type> *);
	typedef int (*<T>biaction_fn)(<type> *, <type> *);
	int <T>compare(const <T> *, const <T> *);
	size_t <T>lower_bound(const <T> *, const <type> *); —access
	size_t <T>upper_bound(const <T> *, const <type> *); —access
	int <T>insert_after(<T> *, const <type> *); —contiguous
	void <T>sort(<T> *); —contiguous
	void <T>reverse(<T> *); —contiguous
	int <T>is_equal(const <T> *, const <T> *);
	void <T>unique_merge(<T> *, const <T>biaction_fn); —contiguous
	void <T>unique(<T> *); —contiguous

I have never used this.
