/** @license 2016 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Parameterised Contiguous Dynamic Array (Vector)

 ![Example of Array](../web/array.png)

 <tag:<T>Array> is a dynamic array that stores contiguous `<T>`, which must be
 set using `ARRAY_TYPE`. To ensure that the capacity is greater then or equal
 to the size, resizing may be necessary and incurs amortised cost.

 `<T>Array` is not synchronised. Errors are returned with `errno`. The
 parameters are preprocessor macros, and are all undefined at the end of the
 file for convenience. `assert.h` is included in this file; to stop the
 debug assertions, use `#define NDEBUG` before `assert.h`.

 @param[ARRAY_NAME, ARRAY_TYPE]
 `<T>` that satisfies `C` naming conventions when mangled and a valid tag-type
 associated therewith; required. `<PT>` is private, whose names are prefixed in
 a manner to avoid collisions; any should be re-defined prior to use elsewhere.

 @param[ARRAY_TO_STRING]
 Optional print function implementing <typedef:<PT>ToString>; makes available
 <fn:<T>ArrayToString>.

 @param[ARRAY_TEST]
 Unit testing framework <fn:<T>ArrayTest>, included in a separate header,
 <../test/ArrayTest.h>. Must be defined equal to a (random) filler function,
 satisfying <typedef:<PT>Action>. Requires `ARRAY_TO_STRING` and not `NDEBUG`.

 @std C89
 @cf [Heap](https://github.com/neil-edelman/Heap)
 @cf [List](https://github.com/neil-edelman/List)
 @cf [Orcish](https://github.com/neil-edelman/Orcish)
 @cf [Pool](https://github.com/neil-edelman/Pool)
 @cf [Set](https://github.com/neil-edelman/Set)
 @cf [Trie](https://github.com/neil-edelman/Trie) */

#include <stdlib.h> /* realloc free */
#include <assert.h> /* assert */
#include <string.h> /* memcpy memmove (strlen) (strerror strcpy memcmp) */
#include <errno.h>  /* errno */

/* Check defines. */
#ifndef ARRAY_NAME
#error Generic ARRAY_NAME undefined.
#endif
#ifndef ARRAY_TYPE
#error Generic ARRAY_TYPE undefined.
#endif
#if defined(ARRAY_TEST) && !defined(ARRAY_TO_STRING)
#error ARRAY_TEST requires ARRAY_TO_STRING.
#endif
#if defined(ARRAY_CHILD) && (defined(ARRAY_TO_STRING) || defined(ARRAY_TEST))
#error With ARRAY_CHILD, defining public interface functions is useless.
#endif
#if defined(T) || defined(T_) || defined(PT_)
#error T, T_, and PT_ cannot be defined.
#endif

/* <Kernighan and Ritchie, 1988, p. 231>. */
#ifdef CAT
#undef CAT
#endif
#ifdef CAT_
#undef CAT_
#endif
#ifdef PCAT
#undef PCAT
#endif
#ifdef PCAT_
#undef PCAT_
#endif
#define CAT_(x, y) x ## y
#define CAT(x, y) CAT_(x, y)
#define PCAT_(x, y) x ## _ ## y
#define PCAT(x, y) PCAT_(x, y)
#define T_(thing) CAT(ARRAY_NAME, thing)
#define PT_(thing) PCAT(array, PCAT(ARRAY_NAME, thing))

/** A valid tag type set by `ARRAY_TYPE`. This becomes `T`. */
typedef ARRAY_TYPE PT_(Type);
#define T PT_(Type)

/** Operates by side-effects. */
typedef void (*PT_(Action))(T *);

/** Returns a boolean given read-only `<T>`. */
typedef int (*PT_(Predicate))(const T *);

/** Returns a boolean given two read-only `<T>`. */
typedef int (*PT_(Bipredicate))(const T *, const T *);

/** Returns a boolean given two `<T>`, specifying the first or second
 argument. */
typedef int (*PT_(Biproject))(T *, T *);

/** Return false, ignore `a` and `b`. @implements <PT>Biproject */
static int PT_(false_project)(T *const a, T *const b)
	{ return (void)a, (void)b, 0; }

/** Manages the array field `first`, which is indexed up to `size`. When
 modifying the topology of this array, it may change memory location to fit;
 any pointers to this memory may become stale. To initialise it to an idle
 state, see <fn:<T>Array>, `ARRAY_IDLE`, `{0}` (`C99`), or being `static`.

 ![States.](../web/states.png) */
struct T_(Array);
/* !first -> !size, first -> capacity >= min && size <= capacity <= max */
struct T_(Array) { T *first; size_t size, capacity; };
/* `{0}` is `C99`. */
#ifndef ARRAY_IDLE /* <!-- !zero */
#define ARRAY_IDLE { 0, 0, 0 }
#endif /* !zero --> */

/** Ensures `min_capacity` of `a`.
 @param[min_capacity] If zero, does nothing.
 @param[update_ptr] Must be in the array or null, it updates this value.
 @return Success; otherwise, `errno` will be set.
 @throws[ERANGE] Tried allocating more then can fit in `size_t` or `realloc`
 doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html).
 @throws[realloc] */
static int PT_(update_reserve)(struct T_(Array) *const a,
	const size_t min_capacity, T **const update_ptr) {
	size_t c0;
	T *first;
	const size_t max_size = (size_t)-1 / sizeof *a->first;
	assert(a);
	if(a->first) {
		if(min_capacity <= a->capacity) return 1;
		c0 = a->capacity, assert(c0 >= 8);
	} else { /* Idle. */
		if(!min_capacity) return 1;
		c0 = 8;
	}
	if(min_capacity > max_size) return errno = ERANGE, 0;
	/* Grow the capacity exponentially, a bit less then Fibonacci. */
	while(c0 < min_capacity) {
		size_t c1 = c0 + (c0 >> 1) + (c0 >> 3);
		if(c0 >= c1) { c0 = max_size; break; } /* Overflow; very unlikely. */
		c0 = c1;
	}
	if(!(first = realloc(a->first, sizeof *a->first * c0)))
		{ if(!errno) errno = ERANGE; return 0; }
	if(update_ptr && a->first != first)
		*update_ptr = first + (*update_ptr - a->first); /* Not strict ISO. */
	a->first = first, a->capacity = c0;
	return 1;
}

/** Call `reserve_update` with `a`, `min_capacity`, and no `update_ptr`; which
 is what usually happens. */
static int PT_(reserve)(struct T_(Array) *const a, const size_t min_capacity)
	{ return PT_(update_reserve)(a, min_capacity, 0); }

/** In `a`, converts `anchor` and `range` à-la-Python and stores them in the
 pointers `p0` and `p1` _st_ `*p0, *p1 \in [0, a.size], *p0 <= *p1`.
 @param[anchor] An element in the array or null to indicate past the end.
 @return Success.
 @throws[ERANGE] `anchor` is not null and not in `a`. `range` is greater then
 +/-65534. `size_t` overflow. */
static int PT_(range)(const struct T_(Array) *const a, const T *anchor,
	const long range, size_t *const p0, size_t *const p1) {
	size_t i0, i1;
	assert(a && p0 && p1);
	if((anchor && (anchor < a->first || anchor >= a->first + a->size))
		|| range > 65534l || range < -65534l) return errno = ERANGE, 0;
	i0 = anchor ? (size_t)(anchor - a->first) : a->size;
	if(range < 0) {
		i1 = (size_t)(-range) > a->size ? 0 : a->size - (size_t)(-range) + 1;
		if(i0 > i1) i1 = i0;
	} else {
		i1 = i0 + (size_t)range;
		if(i0 > i1) return errno = ERANGE, 0;
		if(i1 > a->size) i1 = a->size;
	}
	*p0 = i0, *p1 = i1;
	return 1;
}

/** `a` indices [`i0`, `i1`) will be replaced with `b`. */
static int PT_(replace)(struct T_(Array) *const a, const size_t i0,
	const size_t i1, const struct T_(Array) *const b) {
	const size_t a_range = i1 - i0, b_range = b ? b->size : 0;
	assert(a && a != b && i0 <= i1 && i1 <= a->size);
	if(a_range < b_range) { /* The output is bigger. */
		const size_t diff = b_range - a_range;
		if(a->size > (size_t)-1 - diff) return errno = ERANGE, 0;
			if(!PT_(reserve)(a, a->size + diff)) return 0;
		memmove(a->first + i1 + diff, a->first + i1,
			(a->size - i1) * sizeof *a->first);
		a->size += diff;
	} else if(b_range < a_range) { /* The output is smaller. */
		memmove(a->first + i0 + b_range, a->first + i1,
			(a->size - i1) * sizeof *a->first);
		a->size -= a_range - b_range;
	}
	if(b) memcpy(a->first + i0, b->first, b->size * sizeof *a->first);
	return 1;
}

/** Calls `predicate` for each consecutive pair of elements in `a` and if true,
 surjects two one according to `merge`.
 @order \O(`a.size`) */
static void PT_(compactify)(struct T_(Array) *const a,
	const PT_(Bipredicate) predicate, const PT_(Biproject) merge) {
	size_t target, from, cursor, choice, next, move;
	const size_t last = a->size;
	int is_first, is_last;
	assert(a && predicate && merge);
	for(target = from = cursor = 0; cursor < last; cursor += next) {
		/* Bijective `[from, cursor)` is moved lazily. */
		for(choice = 0, next = 1; cursor + next < last
			&& predicate(a->first + cursor + choice, a->first + cursor + next);
			next++) if(merge(a->first + choice, a->first + next)) choice = next;
		if(next == 1) continue;
		/* Must move injective `cursor + choice \in [cursor, cursor + next)`. */
		is_first = !choice;
		is_last  = (choice == next - 1);
		move = cursor - from + is_first;
		memmove(a->first + target, a->first + from, sizeof *a->first * move),
			target += move;
		if(!is_first && !is_last) memcpy(a->first + target,
			a->first + cursor + choice, sizeof *a->first), target++;
		from = cursor + next - is_last;
	}
	/* Last differed move. */
	move = last - from;
	memmove(a->first + target, a->first + from, sizeof *a->first * move),
		target += move, assert(a->size >= target);
	a->size = target;
}

/** Returns a new un-initialized datum of `a` and updates `update_ptr`. */
static T *PT_(update_new)(struct T_(Array) *const a, T **const update_ptr) {
	assert(a);
	return PT_(update_reserve)(a, a->size + 1, update_ptr)
		? a->first + a->size++ : 0;
}

/** Returns a new un-initialized datum of `a`. */
static T *PT_(new)(struct T_(Array) *const a) {
	assert(a);
	return PT_(reserve)(a, a->size + 1) ? a->first + a->size++ : 0;
}

/** Initialises `a` to idle. */
static void PT_(array)(struct T_(Array) *const a)
	{ assert(a), a->first = 0, a->capacity = a->size = 0; }

/** Destroys `a` and returns it to idle. */
static void PT_(array_)(struct T_(Array) *const a)
	{ assert(a), free(a->first), PT_(array)(a); }

#ifndef ARRAY_CHILD /* <!-- !sub-type */

/** Returns `a` to the idle state where it takes no dynamic memory.
 @param[a] If null, does nothing. @order \Theta(1) @allow */
static void T_(Array_)(struct T_(Array) *const a) { if(a) PT_(array_)(a); }

/** Initialises `a` to be idle. @order \Theta(1) @allow */
static void T_(Array)(struct T_(Array) *const a) { if(a) PT_(array)(a); }

/** Removes `datum` from `a`.
 @param[a, datum] If null, returns false.
 @return Success, otherwise `errno` will be set for valid input.
 @throws[EDOM] `datum` is not part of `a`. @order \O(n). @allow */
static int T_(ArrayRemove)(struct T_(Array) *const a, T *const datum) {
	size_t n;
	if(!a || !datum) return 0;
	if(datum < a->first
		|| (n = datum - a->first) >= a->size) return errno = EDOM, 0;
	memmove(datum, datum + 1, sizeof *datum * (--a->size - n));
	return 1;
}

/** Removes `datum` from `a` and replaces it with the tail.
 @param[a, datum] If null, returns false.
 @return Success, otherwise `errno` will be set for valid input.
 @throws[EDOM] `datum` is not part of `a`. @order \O(1). @allow */
static int T_(ArrayLazyRemove)(struct T_(Array) *const a, T *const datum) {
	size_t n;
	if(!a || !datum) return 0;
	if(datum < a->first
	   || (n = datum - a->first) >= a->size) return errno = EDOM, 0;
	if(--a->size != n) memcpy(datum, a->first + a->size, sizeof *datum);
	return 1;
}

/** Sets `a` to be empty. That is, the size of `a` will be zero, but if it was
 previously in an active non-idle state, it continues to be.
 @param[a] If null, does nothing. @order \Theta(1) @allow */
static void T_(ArrayClear)(struct T_(Array) *const a)
	{ if(a) a->size = 0; }

/** @param[a] If null, returns null.
 @return The last element or null if the a is empty.
 @order \Theta(1) @allow */
static T *T_(ArrayPeek)(const struct T_(Array) *const a)
	{ return a && a->size ? a->first + a->size - 1 : 0; }

/** The same value as <fn:<T>ArrayPeek>.
 @param[a] If null, returns null.
 @return Value from the the top of the `a` that is removed or null if the
 stack is empty.
 @order \Theta(1)
 @allow */
static T *T_(ArrayPop)(struct T_(Array) *const a)
	{ return a && a->size ? a->first + --a->size : 0; }

/** @param[a] If is null, returns null.
 @return A new, un-initialised, element at the back of `a`, or null and `errno`
 will be set.
 @throws[ERANGE] Tried allocating more then can fit in `size_t` or `realloc`
 error and doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html).
 @throws[realloc] @order Amortised \O(1). @allow */
static T *T_(ArrayNew)(struct T_(Array) *const a)
	{ return a ? PT_(new)(a) : 0; }

/** @param[a] If null, returns null.
 @param[update_ptr] Pointer to update on memory move if it is within the memory
 region that was changed to accommodate new space.
 @return A new, un-initialised, element at the back of `a`, or null and `errno`
 will be set.
 @throws[ERANGE] Tried allocating more then can fit in `size_t` or `realloc`
 error and doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html).
 @throws[realloc] @order Amortised \O(1). @allow */
static T *T_(ArrayUpdateNew)(struct T_(Array) *const a,
	T **const update_ptr) { return a ? PT_(update_new)(a, update_ptr) : 0; }

/** Ensures that `a` is `reserve` capacity beyond the elements in the array.
 @param[a] If null, returns null.
 @return The previous end of `a`, where are `reserve` elements, or null
 and `errno` will be set. Writing on this memory space is safe on success, up
 to `reserve` elements, but one will have to increase the size, (see
 <fn:<T>ArrayBuffer>.)
 @throws[ERANGE] Tried allocating more then can fit in `size_t` or `realloc`
 error and doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html).
 @throws[realloc] @allow */
static T *T_(ArrayReserve)(struct T_(Array) *const a, const size_t reserve) {
	if(!a) return 0;
	if(!reserve) return a->first ? a->first + a->size : 0;
	if(a->size > (size_t)-1 - reserve) { errno = ERANGE; return 0; }
	if(!PT_(reserve)(a, a->size + reserve)) return 0;
	assert(a->first);
	return a->first + a->size;
}

/** Adds `add` elements to `a`.
 @param[a] If null, returns null.
 @param[add] If zero, returns null.
 @return The start of a new sub-array of `add` elements at the previous end of
 `a`, or null and `errno` will be set.
 @throws[ERANGE] Tried allocating more then can fit in `size_t` or `realloc`
 error and doesn't follow [POSIX
 ](https://pubs.opengroup.org/onlinepubs/009695399/functions/realloc.html). If
 <fn:<T>ArrayReserve> has been successful in reserving at least `add` elements,
 one is guaranteed success.
 @throws[realloc] @order Amortised \O(`add`). @allow */
static T *T_(ArrayBuffer)(struct T_(Array) *const a, const size_t add) {
	size_t prev_size;
	if(!a || !add) return 0;
	if(a->size > (size_t)-1 - add) { errno = ERANGE; return 0; } /* Unlikely. */
	if(!PT_(reserve)(a, a->size + add)) return 0;
	prev_size = a->size;
	a->size += add;
	return a->first + prev_size;
}

/** Iterates through `a` and calls `action` on all the elements. The topology
 of the list should not change while in this function.
 @param[a, action] If null, does nothing.
 @order \O(`size` \times `action`) @allow */
static void T_(ArrayEach)(struct T_(Array) *const a,
	const PT_(Action) action) {
	T *t, *end;
	if(!a || !action) return;
	for(t = a->first, end = t + a->size; t < end; t++) action(t);
}

/** Iterates through `a` and calls `action` on all the elements for which
 `predicate` returns true. The topology of the list can not change while in
 this function.
 @param[a, predicate, action] If null, does nothing.
 @order \O(`size` \times `action`) @allow */
static void T_(ArrayIfEach)(struct T_(Array) *const a,
	const PT_(Predicate) predicate, const PT_(Action) action) {
	T *t, *end;
	if(!a || !action || !predicate) return;
	for(t = a->first, end = t + a->size; t < end; t++)
		if(predicate(t)) action(t);
}

/** Iterates through `a` and calls `predicate` until it returns true.
 @param[a, predicate] If null, returns null.
 @return The first `predicate` that returned true, or, if the statement is
 false on all, null. @order \O(`size` \times `predicate`) @allow */
static T *T_(ArrayAny)(const struct T_(Array) *const a,
	const PT_(Predicate) predicate) {
	T *t, *end;
	if(!a || !predicate) return 0;
	for(t = a->first, end = t + a->size; t < end; t++)
		if(predicate(t)) return t;
	return 0;
}

/** For all elements of `a`, calls `keep`, and for each element, if the return
 value is false, lazy deletes that item, calling `destruct` if not-null.
 @param[a, keep] If null, does nothing.
 @order \O(`size`)
 @allow */
static void T_(ArrayKeepIf)(struct T_(Array) *const a,
	const PT_(Predicate) keep, const PT_(Action) destruct) {
	T *erase = 0, *t;
	const T *retain = 0, *end;
	int keep0 = 1, keep1 = 0;
	if(!a || !keep) return;
	for(t = a->first, end = a->first + a->size; t < end; keep0 = keep1, t++) {
		if(!(keep1 = !!keep(t)) && destruct) destruct(t);
		if(!(keep0 ^ keep1)) continue; /* Not a falling/rising edge. */
		if(keep1) { /* Rising edge. */
			assert(erase && !retain);
			retain = t;
		} else if(erase) { /* Falling edge. */
			size_t n = t - retain;
			assert(erase < retain && retain < t);
			memmove(erase, retain, n * sizeof *t);
			erase += n;
			retain = 0;
		} else { /* Falling edge, (first time only.) */
			erase = t;
		}
	}
	if(!erase) return; /* All elements were kept. */
	if(keep1) { /* Delayed move when the iteration ended; repeat. */
		size_t n = t - retain;
		assert(retain && erase < retain && retain < t);
		memmove(erase, retain, n * sizeof *t);
		erase += n;
	}
	/* Adjust the size. */
	assert((size_t)(erase - a->first) <= a->size);
	a->size = erase - a->first;
}

/** Calls `predicate` for each consecutive pair of elements in `a` and if true,
 surjects two one according to `merge`.
 @param[a, predicate] If null, does nothing.
 @param[merge] Can be null, in which case the second argument is erased.
 @order \O(`a.size`) */
static void T_(ArrayCompactify)(struct T_(Array) *const a,
	const PT_(Bipredicate) predicate, const PT_(Biproject) merge)
	{ if(a && predicate) PT_(compactify)(a, predicate,
	merge ? merge : &PT_(false_project)); }

/** Removes at either end of `a` of things that `predicate` returns true.
 @param[a, predicate] If null, does nothing.
 @order \O(`size`)
 @allow */
static void T_(ArrayTrim)(struct T_(Array) *const a,
	const PT_(Predicate) predicate) {
	size_t i;
	if(!a || !predicate) return;
	while(a->size && predicate(a->first + a->size - 1)) a->size--;
	for(i = 0; i < a->size && predicate(a->first + i); i++);
	if(!i) return;
	assert(i < a->size);
	memmove(a->first, a->first + i, sizeof *a->first * i), a->size -= i;
}

/** In `a`, replaces the elements from `anchor` up to `range` with a copy of
 `b`.
 @param[a] If null, returns zero.
 @param[anchor] Beginning of the replaced value, inclusive. If null, appends to
 the end.
 @param[range] How many replaced values in the original; negative values are
 implicitly plus the length of the array; clamped at the minimum and maximum.
 @param[b] The replacement array. If null, deletes without replacing. It is
 more efficient than individual <fn:<T>ArrayRemove> to delete several
 consecutive values.
 @return Success.
 @throws[EDOM] `a` and `b` are not null and the same.
 @throws[ERANGE] `anchor` is not null and not in `a`.
 @throws[ERANGE] `range` is greater then 65535 or smaller then -65534.
 @throws[ERANGE] `b` would cause the array to overflow.
 @throws[realloc]
 @order \Theta(`b.size`) if the elements have the same size, otherwise,
 amortised \O(`a.size` + `b.size`).
 @allow */
static int T_(ArraySplice)(struct T_(Array) *const a, const T *anchor,
	const long range, const struct T_(Array) *const b) {
	size_t i0, i1;
	if(!a) return 0;
	if(a == b) return errno = EDOM, 0;
	if(!PT_(range)(a, anchor, range, &i0, &i1)) return 0;
	return PT_(replace)(a, i0, i1, b);
}

/** In `a`, replaces the elements from indices `i0` (inclusive) to `i1`
 (exclusive) with a copy of `b`.
 @param[a] If null, returns zero.
 @param[i0, i1] The replacement indices, `[i0, i1)`, such that
 `0 <= i0 <= i1 <= a.size`.
 @param[b] The replacement array. If null, deletes without replacing.
 @return Success.
 @throws[EDOM] `a` and `b` are not null and the same.
 @throws[EDOM] `i0` or `i1` are out-of-bounds or `i0 > i1`.
 @throws[ERANGE] `b` would cause the array to overflow.
 @throws[realloc]
 @order \Theta(`b.size`) if the elements have the same size, otherwise,
 amortised \O(`a.size` + `b.size`).
 @allow */
static int T_(ArrayIndexSplice)(struct T_(Array) *const a, const size_t i0,
	const size_t i1, const struct T_(Array) *const b) {
	if(!a) return 0;
	if(a == b || i0 > i1 || i1 > a->size) return errno = EDOM, 0;
	return PT_(replace)(a, i0, i1, b);
}

#ifdef ARRAY_TO_STRING /* <!-- string */

/** Responsible for turning the first argument into a 12-`char` null-terminated
 output string. Used for `ARRAY_TO_STRING`. */
typedef void (*PT_(ToString))(const T *, char (*)[12]);
/* Check that `ARRAY_TO_STRING` is a function implementing
 <typedef:<PT>ToString>. */
static const PT_(ToString) PT_(to_string) = (ARRAY_TO_STRING);
	
/** Can print 4 things at once before it overwrites. One must a
 `ARRAY_TO_STRING` to a function implementing <typedef:<PT>ToString> to get
 this functionality.
 @return Prints `a` in a static buffer. @order \Theta(1) @allow */
static const char *T_(ArrayToString)(const struct T_(Array) *const a) {
	static char buffers[4][256];
	static size_t buffer_i;
	char *const buffer = buffers[buffer_i++], *b = buffer;
	const size_t buffers_no = sizeof buffers / sizeof *buffers,
		buffer_size = sizeof *buffers / sizeof **buffers;
	const char start = '(', comma = ',', space = ' ', end = ')',
		*const ellipsis_end = ",…)", *const null = "null",
		*const idle = "idle";
	const size_t ellipsis_end_len = strlen(ellipsis_end),
		null_len = strlen(null), idle_len = strlen(idle);
	size_t i;
	PT_(Type) *e, *e_end;
	int is_first = 1;
	assert(!(buffers_no & (buffers_no - 1)) && ellipsis_end_len >= 1
		&& buffer_size >= 1 + 11 + ellipsis_end_len + 1
		&& buffer_size >= null_len + 1
		&& buffer_size >= idle_len + 1);
	/* Advance the buffer for next time. */
	buffer_i &= buffers_no - 1;
	if(!a) { memcpy(b, null, null_len), b += null_len; goto terminate; }
	if(!a->first) { memcpy(b, idle, idle_len), b += idle_len; goto terminate; }
	*b++ = start;
	for(e = a->first, e_end = a->first + a->size; ; ) {
		if(!is_first) *b++ = comma, *b++ = space;
		else is_first = 0;
		PT_(to_string)(e, (char (*)[12])b);
		for(i = 0; *b != '\0' && i < 12; b++, i++);
		if(++e >= e_end) break;
		if((size_t)(b - buffer) > buffer_size - 2 - 11 - ellipsis_end_len - 1)
			goto ellipsis;
	}
	*b++ = end;
	goto terminate;
ellipsis:
	memcpy(b, ellipsis_end, ellipsis_end_len), b += ellipsis_end_len;
terminate:
	*b++ = '\0';
	assert(b <= buffer + buffer_size);
	return buffer;
}
#endif /* string --> */

#ifdef ARRAY_TEST /* <!-- test: need this file. */
#include "../test/TestArray.h" /** \include */
#endif /* test --> */

static void PT_(unused_coda)(void);
/** This silences unused function warnings from the pre-processor, but allows
 optimisation <http://stackoverflow.com/questions/43841780/silencing-unused-static-function-warnings-for-a-section-of-code>. */
static void PT_(unused_set)(void) {
	T_(Array_)(0), T_(Array)(0), T_(ArrayRemove)(0, 0),
		T_(ArrayLazyRemove)(0, 0), T_(ArrayClear)(0), T_(ArrayPeek)(0),
		T_(ArrayPop)(0), T_(ArrayNew)(0), T_(ArrayUpdateNew)(0, 0),
		T_(ArrayReserve)(0, 0), T_(ArrayBuffer)(0, 0), T_(ArrayEach)(0, 0),
		T_(ArrayIfEach)(0, 0, 0), T_(ArrayAny)(0, 0), T_(ArrayKeepIf)(0, 0, 0),
		T_(ArrayCompactify)(0, 0, 0), T_(ArrayTrim)(0, 0),
		T_(ArraySplice)(0, 0, 0, 0), T_(ArrayIndexSplice)(0, 0, 0, 0);
#ifdef ARRAY_TO_STRING
	T_(ArrayToString)(0);
#endif
	PT_(unused_coda)();
}
/** Some newer compilers are smart. */
static void PT_(unused_coda)(void) { PT_(unused_set)(); }

/* Un-define macros. */
#undef CAT
#undef CAT_
#undef PCAT
#undef PCAT_
#else /* !sub-type --><!-- sub-type */
#undef ARRAY_CHILD
static void PT_(unused_coda)(void);
/** This is a subtype of another, more specialised type. `CAT`, _etc_, have to
 have the same meanings; they will be replaced with these, and `T` cannot be
 used. */
static void PT_(unused_set)(void) {
	/* <fn:<PT>update_reserve> is integral. */
	PT_(false_project)(0, 0), PT_(reserve)(0, 0), PT_(range)(0, 0, 0, 0, 0),
		PT_(replace)(0, 0, 0, 0), PT_(compactify)(0, 0, 0),
		PT_(update_new)(0, 0), PT_(new)(0), PT_(array)(0), PT_(array_)(0),
		PT_(unused_coda)();
}
static void PT_(unused_coda)(void) { PT_(unused_set)(); }
#endif /* sub-type --> */
#undef T
#undef T_
#undef PT_
#undef ARRAY_NAME
#undef ARRAY_TYPE
#ifdef ARRAY_TO_STRING
#undef ARRAY_TO_STRING
#endif
#ifdef ARRAY_TEST
#undef ARRAY_TEST
#endif
