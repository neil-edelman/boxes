# Documentation

* [array](array.md)
* [bmp](bmp.md)
* [heap](heap.mb) —requires array
* [list](list.md) —never used
* [pool](pool.md) —requires array, heap
* [table](table.md)
* [tree](tree.md)
* [trie](trie.md) —requires bmp

Maybe deque will be useful? a linked list of nodes of increasing size, starts at 64, starts at
1/3 the way in.
Maybe we should have a `box.h` to store shared code? It's not much. Maybe error
reporting goes there.

`C90` requires identifiers to be a minimum of 31 significant initial characters in an internal
identifier or a macro name. If your compiler is `C89` and does the very minimum, it
probably won't be enough to differentiate all the private symbols. One may have to
abbreviate.

# Symbols

## Symbols you can set

	BOX_DECLARE_ONLY
In a header.

## Symbols used internally

	BOX_H
All boxes have this and is persistent to the end of the translation unit.

	BOX_RESTRICT
If `C90`, restrict is ignored for all the definitions until the box is done.

	BOX_MINOR_NAME, BOX_MINOR, BOX_MAJOR_NAME, BOX_MAJOR
_Eg_ _foo_, private `typedef` to `strut foo`, _array_, `struct foo_array`. Stays until the box is done.

	BOX_ACCESS, BOX_CONTIGUOUS
Flags. Stays until the box is done.

	typedef PT_(box), PT_(type)

# Interfaces

## view

Generally, a subset of contiguous elements built on top of a container, stored in O(1) space, valid until a topological modification of the container.

Ideas:
> iterator cursor look range subset span [view] (means similar in sql): unbounded laden full [entire], laden occupied [exists], distance size count, get acquire look first begin [front], last end back, shift pop_front (sounds like modifying data) advance (doesn't fit with front) (I kind of like \_front) ditch_ dump_ can_ oust_ expel_ [drop_front] cut_ drain_ eject_ shed_, pop pop_back reverse retract regress recede withdraw retire recede… random?, delete, insert

	struct <T>view; —iterator, cursor
	<T>view <T>entire(const *<T>); —entire container
	int <T>exists(const <T>view); —not empty
	<T> *begin(const <T>view); —must exists
	void <T>drop_begin(<T>view); —must exists; resets exists
	<T> *end(const <T>view); —if back/end is a thing
	void <T>drop_end(<T>view);
	<T>view <T>prefix(const *<T>); —all kinds of constructors
	//<T> *new_front(<T>view); —maybe we should have a new one to go between

General implementation:
*box: !box -> done
front: Where applicable, if the front is null, it should start at the first element automatically for maximum laziness.
back: Where applicable. should be symmetric.

	for(view v = entire(&container); exists(&v);
		drop_front(&v)) print front(&v);

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

# Structure of a box.h

Comment for documentation.

	/** @license 2016 Neil Edelman, distributed under the terms of the
	[MIT License](https://opensource.org/licenses/MIT).
	@abstract …

	@subtitle … */

Checking for (obvious) misconfigurations; remember that it's probably going to be called under different contexts.

	#if !defined(BOX_NAME) || !defined(BOX_TYPE)
	#error Name or tag type undefined.
	#endif
	…
	#if defined(BOX_TEST) && (!defined(BOX_TRAIT) && !defined(BOX_TO_STRING) \
		|| defined(BOX_TRAIT) && !defined(BOX_HAS_TO_STRING))
	#error Test requires to string.
	#endif
	#if defined BOX_HEAD && (defined BOX_BODY || defined BOX_TRAIT)
	#error Can not be simultaneously defined.
	#endif

idempotent includes and setup.

	#ifndef BOX_H /* <!-- idempotent */
	#define BOX_H
	#include <stdlib.h>
	#include <errno.h>
	#include <assert.h>
	#if defined(BOX_CAT_) || defined(BOX_CAT) || defined(B_) || defined(PB_)
	#error Unexpected defines.
	#endif
	/* <Kernighan and Ritchie, 1988, p. 231>. */
	#define BOX_CAT_(n, m) n ## _ ## m
	#define BOX_CAT(n, m) BOX_CAT_(n, m)
	#define B_(n) BOX_CAT(BOX_NAME, n)
	#define PB_(n) BOX_CAT(box, B_(n))
	#endif /* idempotent --> */

