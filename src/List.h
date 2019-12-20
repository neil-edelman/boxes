/** @license 2017 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Parameterised List

 <tag:<L>List> is a list of <tag:<L>ListLink>; it may be supplied a total-order
 function, `LIST_COMPARE` <typedef:<PI>Compare>.

 Internally, `<L>ListLink` is a doubly-linked node with sentinels residing in
 `<L>List`. It only provides an order, but `<L>ListLink` may be enclosed in
 another `struct`. While in the list, the links should not be added to another
 list.

 `<L>Link` is not synchronised. Errors are returned with `errno`. The
 parameters are `#define` preprocessor macros, and are all undefined at the end
 of the file for convenience. `assert.h` is used; to stop assertions, use
 `#define NDEBUG` before inclusion.

 @param[LIST_NAME]
 `<L>` that satisfies `C` naming conventions when mangled; required. `<PL>` is
 private, whose names are prefixed in a manner to avoid collisions; any should
 be re-defined prior to use elsewhere.

 @param[LIST_COMPARE]
 Optional total-order function satisfying <typedef:<PL>Compare>.

 @param[LIST_TO_STRING]
 Optional print function implementing <typedef:<PL>ToString>; makes available
 <fn:<L>ListToString>.

 @param[LIST_TEST]
 Unit testing framework <fn:<L>ListTest>, included in a separate header,
 <../test/TestList.h>. Must be defined equal to a random filler function,
 satisfying <typedef:<PL>Action>. Requires `LIST_TO_STRING` and not `NDEBUG`.

 @std C89
 @cf [Array](https://github.com/neil-edelman/Array)
 @cf [Orcish](https://github.com/neil-edelman/Orcish)
 @cf [Pool](https://github.com/neil-edelman/Pool)
 @cf [Set](https://github.com/neil-edelman/Set) */

#include <stdio.h> /* fixme */

#include <assert.h>
#ifdef LIST_TO_STRING /* <!-- string */
#include <string.h> /* strlen */
#endif /* string --> */

/* Check defines. */
#ifndef LIST_NAME
#error Generic LIST_NAME undefined.
#endif
#if defined(LIST_TEST) && !defined(LIST_TO_STRING)
#error LIST_TEST requires LIST_TO_STRING.
#endif

/* Generics using the preprocessor;
 <http://stackoverflow.com/questions/16522341/pseudo-generics-in-c>. */
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
#ifdef L_
#undef L_
#endif
#ifdef PL_
#undef PL_
#endif
#define CAT_(x, y) x ## y
#define CAT(x, y) CAT_(x, y)
#define PCAT_(x, y) x ## _ ## y
#define PCAT(x, y) PCAT_(x, y)
#define L_(thing) CAT(LIST_NAME, thing)
#define PL_(thing) PCAT(set, PCAT(LIST_NAME, thing)) /* "Private." */

/** Storage of this structure is the responsibility of the caller. */
struct L_(ListLink);
struct L_(ListLink) { struct L_(ListLink) *prev, *next; };

/** Serves as head and tail for linked-list of <tag:<L>ListLink>. Use
 <fn:<L>ListClear> or statically initialise using the macro
 `LIST_IDLE[_[2-4]](<list>)`, depending on how many orders that are in the
 list. Because this list is closed; that is, given a valid pointer to an
 element, one can determine all others, null values are not allowed and it is
 _not_ the same as `{0}`. _Eg_, `struct IntList ints = LIST_IDLE(ints);`. */
struct L_(List);
struct L_(List) {
	/* These are sentinels such that `head.prev` and `tail.next` are always and
	 the only ones to be null. */
	struct L_(ListLink) head, tail;
};
#define LIST_IDLE(l) { { 0, &(l).tail }, { &(l).head, 0 } }

#ifdef LIST_TO_STRING /* <!-- string */
/** Responsible for turning <typedef:<L>ListLink> (the first argument) into a
 maximum 11-`char` string (the second.) */
typedef void (*PL_(ToString))(const struct L_(ListLink) *, char (*)[12]);
/* Check that `SET_TO_STRING` is a function implementing
 <typedef:<PE>ToString>. */
static const PL_(ToString) PL_(to_string) = (LIST_TO_STRING);
#endif /* string --> */

#ifdef SET_TEST /* <!-- test */
/** Used for `SET_TEST`. */
typedef void (*PE_(Action))(PE_(Type) *const);
#endif /* test --> */


/** "Floyd's" tortoise-hare algorithm for cycle detection when in debug mode. */
static void PL_(valid)(struct L_(ListLink) *link) {
#ifdef LIST_TEST
	size_t fw = 0, b1 = 0, b2 = 0;
	struct L_(ListLink) *hare = link, *turtle = hare;
	assert(link);
	for(turtle = hare; hare->prev; hare = hare->prev) {
		if(b1++ & 1) turtle = turtle->prev;
		assert(turtle != hare);
	}
	for(turtle = hare; hare->next; hare = hare->next) {
		if(fw++ & 1) turtle = turtle->next;
		assert(turtle != hare);
	}
	for(turtle = hare; hare != link; hare = hare->prev) {
		if(b2++ & 1) turtle = turtle->prev;
		assert(hare && turtle != hare);
	}
	assert(fw == b1 + b2 + 1);
	fprintf(stderr, "%lu\n", fw);
#else
	(void)(list);
#endif
}

/** Private: clears and initialises `list`. */
static void PL_(clear)(struct L_(List) *const list) {
	assert(list);
	list->head.prev = list->tail.next = 0;
	list->head.next = &list->tail;
	list->tail.prev = &list->head;
}

/** Private: `add` before `anchor`. */
static void PL_(add_before)(struct L_(ListLink) *const anchor,
	struct L_(ListLink) *const add) {
	assert(anchor && add && anchor != add && anchor->prev);
	add->prev = anchor->prev;
	add->next = anchor;
	anchor->prev->next = add;
	anchor->prev = add;
	PL_(valid)(add);
}

/** Private: `add` after `anchor`. */
static void PL_(add_after)(struct L_(ListLink) *const anchor,
	struct L_(ListLink) *const add) {
	assert(anchor && add && anchor != add && anchor->next);
	add->prev = anchor;
	add->next = anchor->next;
	anchor->next->prev = add;
	anchor->next = add;
	PL_(valid)(add);
}

/** Private: `remove`. */
static void PL_(remove)(struct L_(ListLink) *const remove) {
	assert(remove && remove->prev && remove->next);
	remove->prev->next = remove->next;
	remove->next->prev = remove->prev;
	remove->prev = remove->next = 0;
}

#if 0

/** Private: cats all {from} in front of {x}, (don't cat {head}, instead
 {head->next}); {from} will be empty after. Careful that {x} is not in {from}
 because that will just erase the list.
 @order \Theta(1) */
static void PL_(x, cat)(struct PT_(X) *const x,
	struct T_(List) *const from) {
	assert(x && from && x->U_(prev) &&
		!from->head.U_(prev) && from->head.U_(next)
		&& from->tail.U_(prev) && !from->tail.U_(next));
	from->head.U_(next)->U_(prev) = x->U_(prev);
	x->U_(prev)->U_(next) = from->head.U_(next);
	from->tail.U_(prev)->U_(next) = x;
	x->U_(prev) = from->tail.U_(prev);
	from->head.U_(next) = &from->tail;
	from->tail.U_(prev) = &from->head;
	PT_U_(cycle, crash)(x);
	PT_U_(cycle, crash)(&from->head);
}

/** Private: when the actual list but not the data changes locations. */
static void PT_U_(list, self_correct)(struct T_(List) *const list) {
	assert(sizeof(PT_(Type)) > 0);
	/* This is a kind of hack relying on {tail, head} to be in packed order in
	 {<T>List} but not in {<T>Link}. */
	if(list->head.U_(next) == list->tail.U_(prev) + 1) {
		list->head.U_(next) = &list->tail;
		list->tail.U_(prev) = &list->head;
	} else {
		list->head.U_(next)->U_(prev) = &list->head;
		list->tail.U_(prev)->U_(next) = &list->tail;
	}
}

#endif



static void L_(ListClear)(struct L_(List) *const list) {
	if(list) PL_(clear)(list);
}
static struct L_(ListLink) *L_(ListFirst)(const struct L_(List) *const list) {
	struct L_(ListLink) *link;
	if(!list) return 0;
	link = list->head.next, assert(link);
	return link->next ? link : 0;
}
static struct L_(ListLink) *L_(ListLast)(const struct L_(List) *const list) {
	struct L_(ListLink) *link;
	if(!list) return 0;
	link = list->tail.prev, assert(link);
	return link->prev ? link : 0;
}
static struct L_(ListLink) *L_(ListNext)(struct L_(ListLink) *link) {
	if(!link) return 0;
	link = link->next;
	return link && link->next ? link : 0;
}
static struct L_(ListLink) *L_(ListPrevious)(struct L_(ListLink) *link) {
	if(!link) return 0;
	link = link->prev;
	return link && link->prev ? link : 0;
}
#ifdef LIST_TO_STRING /* <!-- string */
/** Can print 2 things at once before it overwrites. One must set
 `LIST_TO_STRING` to a function implementing <typedef:<PL>ToString> to get this
 functionality.
 @return Prints `list` in a static buffer.
 @order \Theta(1); it has a 1024 character limit; every element takes some.
 @allow */
static const char *L_(ListToString)(const struct L_(List) *const list) {
	static char buffers[2][1024];
	static size_t buffer_i;
	char *buffer = buffers[buffer_i++], *b = buffer;
	const size_t buffers_no = sizeof buffers / sizeof *buffers,
		buffer_size = sizeof *buffers / sizeof **buffers;
	const char space = ' ', start = '(', comma = ',', end = ')',
		*const ellipsis_end = ",…)", *const null = "null";
	const size_t ellipsis_end_len = strlen(ellipsis_end),
		null_len = strlen(null);
	struct L_(ListLink) *link;
	size_t i;
	int is_first = 1;
	assert(!(buffers_no & (buffers_no - 1)) && ellipsis_end_len >= 2
		   && buffer_size >= 2 + 11 + ellipsis_end_len + 1
		   && buffer_size >= null_len + 1);
	/* Advance the buffer for next time. */
	buffer_i &= buffers_no - 1;
	/* Null set. */
	if(!list) { memcpy(b, null, null_len), b += null_len; goto terminate; }
	/* Otherwise */
	*b++ = start;
	for(link = L_(ListFirst)(list); link; link = L_(ListNext)(link)) {
		if(is_first) *b++ = space, is_first = 0;
		else *b++ = comma, *b++ = space;
		PL_(to_string)(link, (char (*)[12])b);
		for(i = 0; *b != '\0' && i < 12; b++, i++);
		/* Greedy can not guarantee another; terminate by ellipsis. */
		if((size_t)(b - buffer) > buffer_size - 2 - 11 - ellipsis_end_len - 1) goto ellipsis;
	}
	/*if(!is_first) *b++ = space;*/
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

#ifdef LIST_TEST /* <!-- test: need this file. */
#include "../test/TestList.h" /** \include */
#endif /* test --> */

static void PL_(unused_coda)(void);
/** This silences unused function warnings from the pre-processor, but allows
 optimisation, (hopefully.)
 <http://stackoverflow.com/questions/43841780/silencing-unused-static-function-warnings-for-a-section-of-code> */
static void PL_(unused_set)(void) {
	L_(ListFirst)(0);
	L_(ListLast)(0);
	L_(ListNext)(0);
	L_(ListPrevious)(0);
	L_(ListClear)(0);
#ifdef LIST_TO_STRING
	L_(ListToString)(0);
#endif
	PL_(unused_coda)();
}
static void PL_(unused_coda)(void) { PL_(unused_set)(); }

/* Un-define all macros. Undocumented: allows nestled inclusion in other .h so
 long as `CAT`, _etc_, are the same meaning and `E_`, _etc_, are not
 clobbered. */
#ifdef LIST_SUBTYPE /* <!-- sub */
#undef LIST_SUBTYPE
#else /* sub --><!-- !sub */
#undef CAT
#undef CAT_
#undef PCAT
#undef PCAT_
#endif /* !sub --> */
#undef L_
#undef PL_
#undef LIST_NAME
#ifdef LIST_COMPARE /* <!-- !compare */
#undef LIST_COMPARE
#endif /* !compare --> */
#ifdef LIST_TO_STRING /* <!-- string */
#undef LIST_TO_STRING
#endif /* string --> */
#ifdef LIST_TEST /* <!-- test */
#undef LIST_TEST
#endif /* test --> */
