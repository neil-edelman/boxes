Documentation for the individual boxes is contained in subfolders.

## Fixme

* `array.h` has been transformed by `ARRAY_NON_STATIC`. Do the others.
* `to_string.h` is awkward… do the thing that is in the file.
* Put prototypes-support into `cdoc`. Include ignoring __*.
* `cdoc` gives errors about not finding stuff, even though it works. Establish firm rules on the current directory. I don't know what I was thinking. Give a clear direction of where it will search.
* Get `cdoc` to recognize `.svg`.
* `cdoc` should be named `boxdoc`? So that people don't confuse this with an actual parser. `cboxdoc`?
* `graph.h` HAS_GRAPH_H? Turns out, graphs are great for all sorts of things, not just testing. Actually, automate it with C++17.
* Maybe deque will be useful? a linked list of nodes of increasing size, starts at 64, starts at 1/3 the way in. `DEQUE_HEAP_ONLY`. This would be great for `cdoc`—cringe on `index_array`.
* Maybe more error reporting goes in `box.h`? I'm hesitant because people have their own way to do it, but a static array of chars for `to_string` is already there.
* Rethink `compare.h` and `iterate.h`. These are the first things I did, and it's problematic. More granularity.
* The private-public forwarding all the functions is super-awkward, way too much work, and not sustainable. I do want one to be able to choose the bare minimum for optimization.
* 1-based height on tree and trie.
* Have trunk (root), contiguous bough, branch, leaf, same in tree and trie.
* Change the font, eww. Can I even do that?

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
