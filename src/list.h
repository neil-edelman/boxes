/** @license 2017 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Stand-alone header <src/list.h>; examples <test/test_list.c>; on a
 compatible workstation, `make` creates the test suite of the examples.

 @subtitle Doubly-linked component

 ![Example of a stochastic skip-list.](../doc/list.png)

 In parlance of <Thareja 2014, Structures>, <tag:<L>list> is a circular
 header, or sentinel, to a doubly-linked list of <tag:<L>listlink>. This is a
 closed structure, such that with with a pointer to any element, it is possible
 to extract the entire list. The links will be generally in a larger container.

 @param[LIST_NAME]
 `<L>` that satisfies `C` naming conventions when mangled; required. `<PL>` is
 private, whose names are prefixed in a manner to avoid collisions.

 @param[LIST_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a trait.

 @param[LIST_COMPARE_NAME, LIST_COMPARE, LIST_IS_EQUAL]
 Compare trait contained in <src/list_coda.h>. An optional mangled name for
 uniqueness and a function implementing either <typedef:<PLC>compare_fn> or
 <typedef:<PLC>bipredicate_fn>.

 @param[LIST_TO_STRING_NAME, LIST_TO_STRING]
 To string trait contained in <src/to_string.h>. An optional mangled name for
 uniqueness and function implementing <typedef:<PSTR>to_string_fn>.

 @std C89 */

#ifndef LIST_NAME
#error Name LIST_NAME undefined.
#endif
#if defined(LIST_TO_STRING_NAME) || defined(LIST_TO_STRING)
#define LIST_TO_STRING_TRAIT 1
#else
#define LIST_TO_STRING_TRAIT 0
#endif
#if defined(LIST_COMPARE_NAME) || defined(LIST_COMPARE) \
	|| defined(LIST_IS_EQUAL)
#define LIST_COMPARE_TRAIT 1
#else
#define LIST_COMPARE_TRAIT 0
#endif
#define LIST_TRAITS LIST_TO_STRING_TRAIT + LIST_COMPARE_TRAIT
#if LIST_TRAITS > 1
#error Only one trait per include is allowed; use LIST_EXPECT_TRAIT.
#endif
#if LIST_TRAITS && !defined(BOX)
#error Trying to define a trait without defining the base datatype.
#endif
#if defined(LIST_TO_STRING_NAME) && !defined(LIST_TO_STRING)
#error LIST_TO_STRING_NAME requires LIST_TO_STRING.
#endif
#if defined(LIST_COMPARE_NAME) \
	&& (!(!defined(LIST_COMPARE) ^ !defined(LIST_IS_EQUAL)))
#error LIST_COMPARE_NAME requires LIST_COMPARE or LIST_IS_EQUAL not both.
#endif

#ifndef LIST_H /* <!-- idempotent */
#define LIST_H
#include <assert.h>
#if defined(LIST_CAT_) || defined(LIST_CAT) || defined(L_) || defined(PL_)
#error Unexpected defines.
#endif
/* <Kernighan and Ritchie, 1988, p. 231>. */
#define LIST_CAT_(n, m) n ## _ ## m
#define LIST_CAT(n, m) LIST_CAT_(n, m)
#define L_(n) LIST_CAT(LIST_NAME, n)
#define PL_(n) LIST_CAT(list, L_(n))
#endif /* idempotent --> */

#if !defined(restrict) && (!defined(__STDC__) || !defined(__STDC_VERSION__) \
	|| __STDC_VERSION__ < 199901L)
#define LIST_RESTRICT /* Undo this at the end. */
#define restrict /* Attribute only in C99+. */
#endif


#if LIST_TRAITS == 0 /* <!-- base code */
#define LIST_BASE


/** Storage of this structure is the responsibility of the caller, who must
 provide a stable pointer while in a list. Generally, one encloses this in a
 host `struct` or `union`. The contents of this structure should be treated as
 read-only while in the list.

 ![States.](../doc/node-states.png) */
struct L_(listlink) { struct L_(listlink) *next, *prev; };

/** Serves as head and tail sentinel for a linked-list of <tag:<L>listlink>.

 ![States.](../doc/states.png) */
struct L_(list);
struct L_(list) {
	union {
		struct { struct L_(listlink) head, *part_of_tail; } as_head;
		struct { struct L_(listlink) *part_of_head, tail; } as_tail;
		struct { struct L_(listlink) *next, *zero, *prev; } flat;
	} u;
};

#define BOX_CONTENT struct L_(listlink) *
/** Is `x` not null? @implements `is_content` */
static int PL_(is_content)(const struct L_(listlink) *const x)
	{ return assert(x ? x->next && x->prev : 1), !!x; }
/* Since this is a permutation, the iteration is defined by none other then
 itself. @implements `forward` */
struct PL_(forward) { const struct L_(listlink) *link; };
/** @return Before `l`. @implements `forward_begin` */
static struct PL_(forward) PL_(forward_begin)(const struct L_(list) *const l)
	{ struct PL_(forward) it; it.link = l ? &l->u.as_head.head : 0; return it; }
/** Move to next `it`. @return Element or null. @implements `forward_next` */
static const struct L_(listlink) *PL_(forward_next)(struct PL_(forward) *const
	it) { struct L_(listlink) *n; assert(it);
	if(!it->link || !(n = it->link->next)) return 0;
	return (it->link = n)->next ? n : 0; }

#define BOX_ITERATOR
/* @implements `iterator` */
struct PL_(iterator) { struct L_(listlink) *link; int dir; };
/** @return Before `l`. @implements `begin` */
static struct PL_(iterator) PL_(begin)(struct L_(list) *const l)
	{ struct PL_(iterator) it; it.link = l ? &l->u.as_head.head : 0;
	it.dir = 0; return it; }
/** @return After `l`. @implements `begin` */
static struct PL_(iterator) PL_(end)(struct L_(list) *const l)
	{ struct PL_(iterator) it; it.link = l ? &l->u.as_tail.tail : 0;
	it.dir = 0; return it; }
/** Advances `it`. @implements `next` */
static struct L_(listlink) *PL_(next)(struct PL_(iterator) *const it) {
	struct L_(listlink) *n; assert(it);
	if(!it->link || !(n = it->link->next)) return 0;
	return (it->link = n)->next ? (it->dir = 1, n) : 0;
}
/** Reverses `it`. @implements `previous` */
static struct L_(listlink) *PL_(previous)(struct PL_(iterator) *const it) {
	struct L_(listlink) *n; assert(it);
	if(!it->link || !(n = it->link->prev)) return 0;
	return (it->link = n)->prev ? (it->dir = -1, n) : 0;
}
/** Removes the element last returned by `it`. (Untested.)
 @return There was an element. @implements `remove` */
static int PL_(remove)(struct PL_(iterator) *const it) {
	struct L_(listlink) *n; assert(0 && it);
	if(!it->dir || !(n = it->link) || !n->next || !n->prev) return 0;
	it->link = it->dir < 0 ? n->prev : n->next;
	n->prev->next = n->next;
	n->next->prev = n->prev;
	n->prev = n->next = 0;
}

/* Box override information. */
#define BOX_ PL_
#define BOX struct L_(list)

/** Cats all `from` (can be null) in front of `after`; `from` will be empty
 after. Careful that `after` is not in `from` because that will just erase the
 list. @order \Theta(1) */
static void PL_(move)(struct L_(list) *restrict const from,
	struct L_(listlink) *restrict const after) {
	assert(from && from->u.flat.next && !from->u.flat.zero && from->u.flat.prev
		&& after && after->prev);
	from->u.flat.next->prev = after->prev;
	after->prev->next = from->u.as_head.head.next;
	from->u.flat.prev->next = after;
	after->prev = from->u.as_tail.tail.prev;
	from->u.flat.next = &from->u.as_tail.tail;
	from->u.flat.prev = &from->u.as_head.head;
}

/* For those who want to directly iterate. */

static const struct L_(listlink) *L_(list_head_c)(const struct L_(list) *const
	list) { const struct L_(listlink) *head; assert(list);
	head = list->u.flat.next;
	return head && head->next ? head : 0;
}
static struct L_(listlink) *L_(list_head)(struct L_(list) *const list) {
	struct L_(listlink) *head; assert(list);
	head = list->u.flat.next;
	return head && head->next ? head : 0;
}
static const struct L_(listlink) *L_(list_tail_c)(const struct L_(list) *const
	list) { const struct L_(listlink) *tail; assert(list);
	tail = list->u.flat.prev;
	return tail && tail->prev ? tail : 0;
}
static struct L_(listlink) *L_(list_tail)(struct L_(list) *const list) {
	struct L_(listlink) *tail; assert(list);
	tail = list->u.flat.prev;
	return tail && tail->prev ? tail : 0;
}
static const struct L_(listlink) *L_(list_previous_c)(const struct L_(listlink)
	*const link) { const struct L_(listlink) *prev;
	return link && (prev = link->prev) && prev->prev ? prev : 0;
}
static struct L_(listlink) *L_(list_previous)(struct L_(listlink) *const link) {
	struct L_(listlink) *prev;
	return link && (prev = link->prev) && prev->prev ? prev : 0;
}
static const struct L_(listlink) *L_(list_next_c)(const struct L_(listlink)
	*const link) { const struct L_(listlink) *next;
	return link && (next = link->next) && next->next ? next : 0;
}
static struct L_(listlink) *L_(list_next)(struct L_(listlink) *const link) {
	struct L_(listlink) *next;
	return link && (next = link->next) && next->next ? next : 0;
}

/** Clear `list`. */
static void PL_(clear)(struct L_(list) *const list) {
	list->u.flat.next = &list->u.as_tail.tail;
	list->u.flat.zero = 0;
	list->u.flat.prev = &list->u.as_head.head;
}
/** Clears and initializes `list`. @order \Theta(1) @allow */
static void L_(list_clear)(struct L_(list) *const list)
	{ assert(list), PL_(clear)(list); }

/** `add` before `anchor` as a new node. @order \Theta(1) */
static void PL_(add_before)(struct L_(listlink) *restrict const anchor,
	struct L_(listlink) *restrict const add) {
	add->prev = anchor->prev;
	add->next = anchor;
	anchor->prev->next = add;
	anchor->prev = add;
}
/** `add` before `anchor`. @order \Theta(1) @allow */
static void L_(list_add_before)(struct L_(listlink) *restrict const anchor,
	struct L_(listlink) *restrict const add) {
	assert(anchor && add && anchor != add && anchor->prev);
	PL_(add_before)(anchor, add);
}

/** `add` after `anchor`. @order \Theta(1) */
static void PL_(add_after)(struct L_(listlink) *restrict const anchor,
	struct L_(listlink) *restrict const add) {
	add->prev = anchor;
	add->next = anchor->next;
	anchor->next->prev = add;
	anchor->next = add;
}
/** `add` after `anchor`. @order \Theta(1) @allow */
static void L_(list_add_after)(struct L_(listlink) *const anchor,
	struct L_(listlink) *const add) {
	assert(anchor && add && anchor != add && anchor->next);
	PL_(add_after)(anchor, add);
}

/** Adds `add` to the end of `list`. @order \Theta(1) */
static void PL_(push)(struct L_(list) *const list,
	struct L_(listlink) *const add)
	{ PL_(add_before)(&list->u.as_tail.tail, add); }
/** Adds `add` to the end of `list`. @order \Theta(1) @allow */
static void L_(list_push)(struct L_(list) *const list,
	struct L_(listlink) *const add)
	{ assert(list && add), PL_(push)(list, add); }

/** Adds `add` to the beginning of `list`. @order \Theta(1) @allow */
static void L_(list_unshift)(struct L_(list) *const list,
	struct L_(listlink) *const add)
	{ assert(list && add), PL_(add_after)(&list->u.as_head.head, add); }

/** Remove `node`. @order \Theta(1) */
static void PL_(rm)(struct L_(listlink) *const node) {
	node->prev->next = node->next;
	node->next->prev = node->prev;
	node->prev = node->next = 0;
}

/** Remove `node`. @order \Theta(1) @allow */
static void L_(list_remove)(struct L_(listlink) *const node)
	{ assert(node && node->prev && node->next), PL_(rm)(node); }

/** Removes the first element of `list` and returns it, if any.
 @order \Theta(1) @allow */
static struct L_(listlink) *L_(list_shift)(struct L_(list) *const list) {
	struct L_(listlink) *node;
	assert(list && list->u.flat.next);
	if(!(node = list->u.flat.next)->next) return 0;
	L_(list_remove)(node);
	return node;
}

/** Removes the last element of `list` and returns it, if any.
 @order \Theta(1) @allow */
static struct L_(listlink) *L_(list_pop)(struct L_(list) *const list) {
	struct L_(listlink) *node;
	assert(list && list->u.flat.prev);
	if(!(node = list->u.flat.prev)->prev) return 0;
	L_(list_remove)(node);
	return node;
}

/** Moves the elements `from` onto `to` at the end.
 @param[to] If null, then it removes elements from `from`.
 @order \Theta(1) @allow */
static void L_(list_to)(struct L_(list) *restrict const from,
	struct L_(list) *restrict const to) {
	assert(from && from != to);
	if(!to) PL_(clear)(from);
	else PL_(move)(from, &to->u.as_tail.tail);
}

/** Moves the elements `from` immediately before `anchor`, which can not be in
 the same list. @order \Theta(1) @allow */
static void L_(list_to_before)(struct L_(list) *restrict const from,
	struct L_(listlink) *restrict const anchor) {
	assert(from && anchor);
	PL_(move)(from, anchor);
}

/** Corrects `list` ends to compensate for memory relocation of the list head
 itself. (Can only have one copy of the list, this will invalidate all other
 copies.) @order \Theta(1) @allow */
static void L_(list_self_correct)(struct L_(list) *const list) {
	assert(list && !list->u.flat.zero);
	if(!list->u.flat.next->next) { /* Empty. */
		assert(!list->u.flat.prev->prev);
		list->u.flat.next = &list->u.as_tail.tail;
		list->u.flat.prev = &list->u.as_head.head;
	} else { /* Non-empty. */
		list->u.flat.prev->next = &list->u.as_tail.tail;
		list->u.flat.next->prev = &list->u.as_head.head;
	}
}

#if 0 /* <!-- superseded by iterate.h; make sure they are equivalent. */
/** Performs `action` for each element in `list` in order.
 @param[action] It makes a double of the next node, so it can be to delete the
 element and even assign it's values null.
 @order \Theta(|`list`|) \times O(`action`) @allow */
static void L_(list_for_each)(struct L_(list) *const list,
	const PL_(action_fn) action) {
	struct L_(listlink) *x, *next_x;
	assert(list && action);
	for(x = list->u.flat.next; next_x = x->next; x = next_x) action(x);
}
/** Iterates through `list` and calls `predicate` until it returns true.
 @return The first `predicate` that returned true, or, if the statement is
 false on all, null.
 @order \O(|`list`|) \times \O(`predicate`) @allow */
static struct L_(listlink) *L_(list_anyy)(const struct L_(list) *const list,
	const PL_(predicate_fn) predicate) {
	struct L_(listlink) *link, *next_link;
	assert(list && predicate);
	for(link = list->u.flat.next; next_link = link->next; link = next_link)
		if(predicate(link)) return link;
	return 0;
}
#endif /* --> */

#ifdef HAVE_ITERATE_H /* <!-- iterate */
#define ITR_(n) LIST_CAT(L_(list), n)
#include "iterate.h" /** \include */
/** HAVE_ITERATE_H: Moves all elements `from` onto the tail of `to` if
 `predicate` is true.
 @param[to] If null, then it removes elements.
 @order \Theta(|`from`|) \times \O(`predicate`) @allow */
static void ITR_(to_if)(struct L_(list) *restrict const from,
	struct L_(list) *restrict const to, const PITR_(predicate_fn) predicate) {
	struct L_(listlink) *link, *next_link;
	assert(from && from != to && predicate);
	for(link = from->u.flat.next; next_link = link->next; link = next_link) {
		if(!predicate(link)) continue;
		L_(list_remove)(link);
		if(to) L_(list_add_before)(&to->u.as_tail.tail, link);
	}
}
#undef ITR_
#endif /* iterate --> */

#ifdef LIST_TEST /* <!-- test */
/* Forward-declare. */
static void (*PL_(to_string))(const struct L_(listlink) *, char (*)[12]);
static const char *(*PL_(list_to_string))(const struct L_(list) *);
#include "../test/test_list.h"
#endif /* test --> */

static void PL_(unused_base_coda)(void);
static void PL_(unused_base)(void) {
	PL_(is_content)(0); PL_(forward_begin)(0); PL_(forward_next)(0);
	PL_(begin)(0); PL_(end)(0); PL_(previous)(0); PL_(next)(0); PL_(remove)(0);
	L_(list_head_c)(0); L_(list_head)(0); L_(list_tail_c)(0); L_(list_tail)(0);
	L_(list_previous_c)(0); L_(list_previous)(0);
	L_(list_next_c)(0); L_(list_next)(0);
	L_(list_clear)(0); L_(list_add_before)(0, 0); L_(list_add_after)(0, 0);
	L_(list_unshift)(0, 0); L_(list_push)(0, 0); L_(list_remove)(0);
	L_(list_shift)(0); L_(list_pop)(0); L_(list_to)(0, 0);
	L_(list_to_before)(0, 0); L_(list_self_correct)(0);
#ifdef HAVE_ITERATE_H
	L_(list_to_if)(0, 0, 0);
#endif
	PL_(unused_base_coda)();
}
static void PL_(unused_base_coda)(void) { PL_(unused_base)(); }


#elif defined(LIST_TO_STRING) /* base code --><!-- to string trait */


#ifdef LIST_TO_STRING_NAME
#define STR_(n) LIST_CAT(L_(list), LIST_CAT(LIST_TO_STRING_NAME, n))
#else
#define STR_(n) LIST_CAT(L_(list), n)
#endif
#define TO_STRING LIST_TO_STRING
#include "to_string.h" /** \include */
#ifdef LIST_TEST /* <!-- expect: greedy satisfy forward-declared. */
#undef LIST_TEST
static PSTR_(to_string_fn) PL_(to_string) = PSTR_(to_string);
static const char *(*PL_(list_to_string))(const struct L_(list) *)
	= &STR_(to_string);
#endif /* expect --> */
#undef STR_
#undef LIST_TO_STRING
#ifdef LIST_TO_STRING_NAME
#undef LIST_TO_STRING_NAME
#endif


#else /* to string trait --><!-- compare trait */


#ifdef LIST_COMPARE_NAME
#define CMP_(n) LIST_CAT(L_(list), LIST_CAT(LIST_COMPARE_NAME, n))
#else
#define CMP_(n) LIST_CAT(L_(list), n)
#endif
#ifdef LIST_COMPARE
#define COMPARE LIST_COMPARE
#else
#define COMPARE_IS_EQUAL LIST_IS_EQUAL
#endif
#include "compare.h" /** \include */
#ifdef LIST_COMPARE /* <!-- compare: override `qsort` -> natural merge sort. */
/** Merges `from` into `to`, preferring elements from `to` go in the front.
 @order \O(|`from`| + |`to`|). @allow */
static void CMP_(merge)(struct L_(list) *restrict const to,
	struct L_(list) *restrict const from) {
	struct L_(listlink) *head, **x = &head, *prev = &to->u.as_head.head, *t, *f;
	assert(to && to->u.flat.next && to->u.flat.prev
		&& from && from->u.flat.next && from->u.flat.prev && from != to);
	/* Empty. */
	if(!(f = from->u.flat.next)->next) return;
	if(!(t = to->u.flat.next)->next)
		{ PL_(move)(from, &to->u.as_tail.tail); return; }
	/* Exclude sentinel. */
	from->u.flat.prev->next = to->u.flat.prev->next = 0;
	/* Merge. */
	for( ; ; ) {
		if(PCMP_(compare)(t, f) <= 0) {
			t->prev = prev, prev = *x = t, x = &t->next;
			if(!(t = t->next)) { *x = f; goto from_left; }
		} else {
			f->prev = prev, prev = *x = f, x = &f->next;
			if(!(f = f->next)) { *x = t; break; }
		}
	}
	if(0) {
from_left:
		f->prev = prev;
		/* Switch sentinels. */
		f = from->u.flat.prev;
		to->u.flat.prev = f;
		f->next = &from->u.as_tail.tail;
	} else {
		t->prev = prev;
	}
	/* Erase `from`. */
	from->u.flat.next = &from->u.as_tail.tail;
	from->u.flat.prev = &from->u.as_head.head;
}
/** Merges the two top runs referenced by `head_ptr` in stack form. */
static void PCMP_(merge_runs)(struct L_(listlink) **const head_ptr) {
	struct L_(listlink) *head = *head_ptr, **x = &head, *b = head, *a = b->prev,
		*const prev = a->prev;
	assert(head_ptr && a && b);
	for( ; ; ) {
		if(PCMP_(compare)(a, b) <= 0) {
			*x = a, x = &a->next;
			if(!(a = a->next)) { *x = b; break; }
		} else {
			*x = b, x = &b->next;
			if(!(b = b->next)) { *x = a; break; }
		}
	}
	head->prev = prev, *head_ptr = head; /* `prev` is the previous run. */
}
/** The list form of `list` is restored from `head` in stack form with two
 runs. */
static void PCMP_(merge_final)(struct L_(list) *const list,
	struct L_(listlink) *head) {
	struct L_(listlink) *prev = 0, **x = &list->u.flat.next,
		*b = head, *a = head->prev;
	assert(list && b && a && !a->prev);
	for( ; ; ) {
		if(PCMP_(compare)(a, b) <= 0) {
			a->prev = prev, prev = *x = a, x = &a->next;
			if(!(a = a->next)) { a = *x = b; break; }
		} else {
			b->prev = prev, prev = *x = b, x = &b->next;
			if(!(b = b->next)) { *x = a; break; }
		}
	}
	do; while(a->prev = prev, prev = a, a = a->next);
	prev->next = &list->u.as_tail.tail, list->u.flat.prev = prev;
	/* Not empty. */
	assert(list->u.flat.next && list->u.flat.next != &list->u.as_tail.tail);
	list->u.flat.next->prev = &list->u.as_head.head;
}
/** `LIST_COMPARE`: Natural merge sort `list`, a stable, adaptive sort,
 according to `compare`. This list-only version is slower then `qsort`.
 @order \Omega(|`list`|), \O(|`list`| log |`list`|) @allow */
static void CMP_(sort)(struct L_(list) *const list) {
	/* Add `[-1,0,1]`: unique identifier for nine weakly-ordered transitions. */
	enum { DEC = 1, EQ = 4, INC = 7 };
	int mono = EQ, cmp;
	struct L_(listlink) *a, *b, *c, *dec_iso = /* Unused. */0;
	struct { size_t count; struct L_(listlink) *head, *prev; } run;
	/* Closed sentinel list. */
	assert(list
		&& list->u.flat.next && !list->u.flat.zero && list->u.flat.prev);
	if(a = list->u.flat.next, !(b = a->next)) return; /* Empty. */
	/* Identify runs of monotonicity until `b` sentinel. */
	run.count = 0, run.prev = 0, run.head = a;
	for(c = b->next; c; a = b, b = c, c = c->next) {
		cmp = PCMP_(compare)(b, a);
		switch(mono + (0 < cmp) - (cmp < 0)) {
			/* Valley and mountain inflection. */
		case INC - 1: a->next = 0; /* _Sic_. */
		case DEC + 1: break;
			/* Decreasing more and levelled off from decreasing. */
		case DEC - 1: b->next = dec_iso; dec_iso = run.head = b; continue;
		case DEC + 0: b->next = a->next; a->next = b; continue;
			/* Turning down and up. */
		case EQ  - 1: a->next = 0; b->next = run.head; dec_iso = run.head = b;
			mono = DEC; continue;
		case EQ  + 1: mono = INC; continue;
		case EQ  + 0: /* Same. _Sic_. */
		case INC + 0: /* Levelled off from increasing. _Sic_. */
		case INC + 1: continue; /* Increasing more. */
		}
		/* Binary carry sequence, <https://oeis.org/A007814>, one delayed so
		 always room for final merge. */
		if(run.count) {
			size_t rc;
			for(rc = run.count - 1; rc & 1; rc >>= 1)
				PCMP_(merge_runs)(&run.prev);
		}
		/* Add to runs, advance; `b` becomes `a` forthwith. */
		run.head->prev = run.prev, run.prev = run.head, run.count++;
		run.head = b, mono = EQ;
	}
	/* Last run; go into an accepting state. */
	if(mono != DEC) {
		if(!run.count) return; /* Sorted already. */
		a->next = 0; /* Last one of increasing or equal. */
	} else { /* Decreasing. */
		assert(dec_iso);
		run.head = dec_iso;
		if(!run.count) { /* Restore the pointers without having two runs. */
			list->u.flat.next = dec_iso, dec_iso->prev = &list->u.as_head.head;
			for(a = dec_iso, b = a->next; b; a = b, b = b->next) b->prev = a;
			list->u.flat.prev = a, a->next = &list->u.as_tail.tail;
			return; /* Reverse sorted; now good as well. */
		}
	}
	assert(run.count);
	/* (Actually slower to merge the last one eagerly. So do nothing.) */
	run.head->prev = run.prev, run.count++;
	/* Merge leftovers from the other direction, saving one for final. */
	while(run.head->prev->prev) PCMP_(merge_runs)(&run.head);
	PCMP_(merge_final)(list, run.head);
}
static void PL_(unused_extra_compare_coda)(void);
static void PL_(unused_extra_compare)(void) {
	CMP_(merge)(0, 0); CMP_(sort)(0); PL_(unused_extra_compare_coda)();
}
static void PL_(unused_extra_compare_coda)(void){ PL_(unused_extra_compare)(); }
#endif /* compare --> */
#ifdef LIST_TEST /* <!-- test: this detects and outputs compare test. */
#include "../test/test_list.h"
#endif /* test --> */
#undef CMP_
#ifdef LIST_COMPARE_NAME
#undef LIST_COMPARE_NAME
#endif
#ifdef LIST_COMPARE
#undef LIST_COMPARE
#endif
#ifdef LIST_IS_EQUAL
#undef LIST_IS_EQUAL
#endif


#endif /* traits --> */


#ifdef LIST_EXPECT_TRAIT /* <!-- trait */
#undef LIST_EXPECT_TRAIT
#else /* trait --><!-- !trait */
#ifdef LIST_TEST
#error No LIST_TO_STRING traits defined for LIST_TEST.
#endif
#undef LIST_NAME
#undef BOX_
#undef BOX
#undef BOX_CONTENT
/*#undef BOX_ITERATOR*/
#endif /* !trait --> */
#undef LIST_TO_STRING_TRAIT
#undef LIST_COMPARE_TRAIT
#undef LIST_TRAITS
#ifdef LIST_RESTRICT
#undef LIST_RESTRICT
#undef restrict
#endif
