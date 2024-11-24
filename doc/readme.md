## view interface

Generally, a subset of contiguous elements built on top of a container, stored in O(1) space, valid until a topological modification of the container.

Ideas:
> iterator cursor look range subset span [view] (means similar in sql): unbounded laden full [entire], laden occupied [exists], distance size count, get acquire look first [front], last back, shift pop_front (sounds like modifying data) advance (doesn't fit with front) (I kind of like \_front) ditch_ dump_ can_ oust_ expel_ [drop_front] cut_ drain_ eject_ shed_, pop pop_back reverse retract regress recede withdraw retire recede… random?, delete, insert

	struct <T>view; —iterator, cursor
	<T>view <T>entire(const *<T>); —entire container
	int <T>exists(const <T>view); —not empty
	void <T>drop_front(<T>view); —must exists; resets exists
	<T> *front(const <T>view); —must exists
	void <T>drop_back(<T>view); —if back is a thing
	<T> *back(const <T>view);
	<T>view <T>prefix(const *<T>); —all kinds of constructors
	<T> *new_front(<T>view); —maybe we should have a new one

General implementation:
*box: !box -> done
front: Where applicable, if the front is null, it should start at the first element automatically for maximum laziness.
back: Where applicable. should be symmetric.

	for(view v = entire(&container); exists(&v);
		drop_front(&v)) print front(&v);

## to_string interface

	view, entire, exists, drop_front, front
	void <type>to_string(const *<type>, char (*)[12]); —supplied by user, [11] is a dud

If turned on by the options, includes `to_string.h` and supplies,

	const char *<T>to_string(const <T> *);

Which calls `<type>to_string` in the order of `entire` `front` and prints in one of 4 static buffers.

## iterate interface

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

## compare interface

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
