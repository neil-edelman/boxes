/** @license 2017 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @abstract Stand-alone header <../src/list.h>; examples <../test/test_list.c>.

 @subtitle Doubly-linked component

 ![Example of a stochastic skip-list.](../doc/list/list.png)

 In parlance of <Thareja 2014, Structures>, <tag:<t>list> is a circular
 header, or sentinel, to a doubly-linked list of <tag:<t>listlink>. This is a
 closed structure, such that with with a pointer to any element, it is possible
 to extract the entire list. The links will be generally in a larger container
 type.

 @param[LIST_NAME]
 `<t>` that satisfies `C` naming conventions when mangled; required. `<PL>` is
 private, whose names are prefixed in a manner to avoid collisions.

 @param[LIST_COMPARE, LIST_IS_EQUAL]
 Compare `<CMP>` trait contained in <src/compare.h>. Requires
 `<name>[<trait>]compare` to be declared as <typedef:<PCMP>compare_fn> or
 `<name>[<trait>]is_equal` to be declared as <typedef:<PCMP>bipredicate_fn>,
 respectfully, (but not both.)

 @param[LIST_TO_STRING]
 To string trait `<STR>` contained in <src/to_string.h>. Require
 `<name>[<trait>]to_string` be declared as <typedef:<PSTR>to_string_fn>.

 @param[LIST_EXPECT_TRAIT, LIST_TRAIT]
 Named traits are obtained by including `array.h` multiple times with
 `LIST_EXPECT_TRAIT` and then subsequently including the name in `LIST_TRAIT`.

 @std C89 */

#ifndef LIST_NAME
#	error Name undefined.
#endif
#if !defined(BOX_ENTRY1) && (defined(LIST_TRAIT) ^ defined(BOX_MAJOR))
#	error LIST_TRAIT name must come after LIST_EXPECT_TRAIT.
#endif
#if defined(LIST_COMPARE) && defined(LIST_IS_EQUAL)
#	error Only one can be defined at a time.
#endif
#if defined(LIST_TEST) && (!defined(LIST_TRAIT) && !defined(LIST_TO_STRING) \
	|| defined(LIST_TRAIT) && !defined(LIST_HAS_TO_STRING))
#	error Test requires to string.
#endif

#ifndef LIST_H /* <!-- idempotent */
#	define LIST_H
enum list_operation { /* Dummy ensures closed. */
	LIST_SUBTRACTION_AB = 1,
	LIST_SUBTRACTION_BA = 2, LISTA,
	LIST_INTERSECTION   = 4, LISTB, LISTC, LISTD,
	LIST_DEFAULT_A      = 8, LISTE, LISTF, LISTG, LISTH, LISTI, LISTJ, LISTK,
	LIST_DEFAULT_B      = 16, LISTL, LISTM, LISTN, LISTO, LISTP, LISTQ, LISTR,
		LISTS, LISTT, LISTU, LISTV, LISTW, LISTX, LISTY, LISTZ
};
#endif /* idempotent --> */

#ifdef LIST_TRAIT
#	define BOX_TRAIT LIST_TRAIT /* Ifdef in <box.h>. */
#endif
#define BOX_START
#include "box.h"

#ifndef LIST_TRAIT /* <!-- base code */
#	include <assert.h>

#	define BOX_MINOR LIST_NAME
#	define BOX_MAJOR list

/** Storage of this structure is the responsibility of the caller, who must
 provide a stable pointer while in a list. Generally, one encloses this in a
 host `struct` or `union`.

 ![States.](../doc/list/node-states.png) */
struct t_(listlink) { struct t_(listlink) *next, *prev; };

/** Serves as head and tail sentinel for a linked-list of <tag:<t>listlink>.

 ![States.](../doc/list/states.png) */
struct t_(list);
struct t_(list) {
	union {
		struct { struct t_(listlink) head, *part_of_tail; } as_head;
		struct { struct t_(listlink) *part_of_head, tail; } as_tail;
		struct { struct t_(listlink) *next, *zero, *prev; } flat;
	} u;
};
typedef struct t_(list) pT_(box);

/* Since this is a permutation, the iteration is defined by none other then
 itself. Not especially useful, but contracted to other files.
 @implements `iterator` */
struct T_(cursor) { struct t_(listlink) *link; };

#	ifndef LIST_DECLARE_ONLY /* Produce code: not for headers. */

/** @return The head of `list` or null. */
static struct t_(listlink) *T_(head)(struct t_(list) *const list) {
	struct t_(listlink) *head; assert(list);
	return (head = list->u.flat.next) && head->next ? head : 0;
}
/** @return The tail of `list` or null. */
static struct t_(listlink) *T_(tail)(struct t_(list) *const list) {
	struct t_(listlink) *tail; assert(list);
	return (tail = list->u.flat.prev) && tail->prev ? tail : 0;
}
/** @return The previous of `link` or null. */
static struct t_(listlink) *T_(link_previous)(struct t_(listlink) *const link) {
	struct t_(listlink) *prev;
	return link && (prev = link->prev) && prev->prev ? prev : 0;
}
/** @return The next of `link` or null. */
static struct t_(listlink) *T_(link_next)(struct t_(listlink) *const link) {
	struct t_(listlink) *next;
	return link && (next = link->next) && next->next ? next : 0;
}
/** @return A pointer to the first in `l` (can be null). */
static struct T_(cursor) T_(begin)(struct t_(list) *const l)
	{ struct T_(cursor) cur; cur.link = l ? T_(head)(l) : 0; return cur; }
/** @return Whether the `cur` points to an element. */
static int T_(exists)(const struct T_(cursor) *const cur)
	{ return cur && cur->link && cur->link->next; }
/** @return Link to `it`, which is just itself. */
static struct t_(listlink) *T_(look)(struct T_(cursor) *const cur)
	{ return cur->link; }
/** @return Advances `it`. */
static void T_(next)(struct T_(cursor) *const cur)
	{ cur->link = cur->link->next; }

/** Cats all `from` (can be null) in front of `after`; `from` will be empty
 after. Careful that `after` is not in `from` because that will just erase the
 list. @order \Theta(1) */
static void pT_(move)(struct t_(list) *restrict const from,
	struct t_(listlink) *restrict const after) {
	assert(from && from->u.flat.next && !from->u.flat.zero && from->u.flat.prev
		&& after && after->prev);
	from->u.flat.next->prev = after->prev;
	after->prev->next = from->u.as_head.head.next;
	from->u.flat.prev->next = after;
	after->prev = from->u.as_tail.tail.prev;
	from->u.flat.next = &from->u.as_tail.tail;
	from->u.flat.prev = &from->u.as_head.head;
}
/** Clear `list`. */
static void pT_(clear)(struct t_(list) *const list) {
	list->u.flat.next = &list->u.as_tail.tail;
	list->u.flat.zero = 0;
	list->u.flat.prev = &list->u.as_head.head;
}
/** Clears and initializes `list`. @order \Theta(1) @allow */
static void T_(clear)(struct t_(list) *const list)
	{ assert(list), pT_(clear)(list); }

/** `add` before `anchor` as a new node. @order \Theta(1) */
static void pT_(add_before)(struct t_(listlink) *restrict const anchor,
	struct t_(listlink) *restrict const add) {
	add->prev = anchor->prev;
	add->next = anchor;
	anchor->prev->next = add;
	anchor->prev = add;
}
/** `add` before `anchor`. @order \Theta(1) @allow */
static void T_(add_before)(struct t_(listlink) *restrict const anchor,
	struct t_(listlink) *restrict const add) {
	assert(anchor && add && anchor != add && anchor->prev);
	pT_(add_before)(anchor, add);
}

/** `add` after `anchor`. @order \Theta(1) */
static void pT_(add_after)(struct t_(listlink) *restrict const anchor,
	struct t_(listlink) *restrict const add) {
	add->prev = anchor;
	add->next = anchor->next;
	anchor->next->prev = add;
	anchor->next = add;
}
/** `add` after `anchor`. @order \Theta(1) @allow */
static void T_(add_after)(struct t_(listlink) *const anchor,
	struct t_(listlink) *const add) {
	assert(anchor && add && anchor != add && anchor->next);
	pT_(add_after)(anchor, add);
}

/** Adds `add` to the end of `list`. @order \Theta(1) */
static void pT_(push)(struct t_(list) *const list,
	struct t_(listlink) *const add)
	{ pT_(add_before)(&list->u.as_tail.tail, add); }
/** Adds `add` to the end of `list`. @order \Theta(1) @allow */
static void T_(push)(struct t_(list) *const list,
	struct t_(listlink) *const add)
	{ assert(list && add), pT_(push)(list, add); }

/** Adds `add` to the beginning of `list`. @order \Theta(1) @allow */
static void T_(unshift)(struct t_(list) *const list,
	struct t_(listlink) *const add)
	{ assert(list && add), pT_(add_after)(&list->u.as_head.head, add); }

/** Remove `node`. @order \Theta(1) */
static void pT_(rm)(struct t_(listlink) *const node) {
	node->prev->next = node->next;
	node->next->prev = node->prev;
	node->prev = node->next = 0;
}

/** Remove `node`. @order \Theta(1) @allow */
static void T_(remove)(struct t_(listlink) *const node)
	{ assert(node && node->prev && node->next), pT_(rm)(node); }

/** Removes the first element of `list` and returns it, if any.
 @order \Theta(1) @allow */
static struct t_(listlink) *T_(shift)(struct t_(list) *const list) {
	struct t_(listlink) *node;
	assert(list && list->u.flat.next);
	if(!(node = list->u.flat.next)->next) return 0;
	pT_(rm)(node);
	return node;
}

/** Removes the last element of `list` and returns it, if any.
 @order \Theta(1) @allow */
static struct t_(listlink) *T_(pop)(struct t_(list) *const list) {
	struct t_(listlink) *node;
	assert(list && list->u.flat.prev);
	if(!(node = list->u.flat.prev)->prev) return 0;
	pT_(rm)(node);
	return node;
}

/** Moves the elements `from` onto `to` at the end.
 @param[to] If null, then it removes elements from `from`.
 @order \Theta(1) @allow */
static void T_(to)(struct t_(list) *restrict const from,
	struct t_(list) *restrict const to) {
	assert(from && from != to);
	if(!to) pT_(clear)(from);
	else pT_(move)(from, &to->u.as_tail.tail);
}

/** Moves the elements `from` immediately before `anchor`, which can not be in
 the same list. @order \Theta(1) @allow */
static void T_(to_before)(struct t_(list) *restrict const from,
	struct t_(listlink) *restrict const anchor) {
	assert(from && anchor);
	pT_(move)(from, anchor);
}

/** Corrects `list` ends to compensate for memory relocation of the list head
 itself. (Can only have one copy of the list, this will invalidate all other
 copies.) @order \Theta(1) @allow */
static void T_(self_correct)(struct t_(list) *const list) {
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

/* fixme: test coverage fails, see table. */
/* fixme: I don't know if this is good? but compare.h and iterate.h both have
 item. */
typedef struct t_(listlink) pT_(type);

#		ifdef HAVE_ITERATE_H /* <!-- iterate */
#			include "iterate.h" /** \include */
/** HAVE_ITERATE_H: Moves all elements `from` onto the tail of `to` if
 `predicate` is true.
 @param[to] If null, then it removes elements.
 @order \Theta(|`from`|) \times \O(`predicate`) @allow */
static void T_(to_if)(struct t_(list) *restrict const from,
	struct t_(list) *restrict const to, const pT_(predicate_fn) predicate) {
	struct t_(listlink) *link, *next_link;
	assert(from && from != to && predicate);
	for(link = from->u.flat.next; next_link = link->next; link = next_link) {
		if(!predicate(link)) continue;
		T_(remove)(link);
		if(to) pT_(add_before)(&to->u.as_tail.tail, link);
	}
}
#		endif /* iterate --> */

static void pT_(unused_base_coda)(void);
static void pT_(unused_base)(void) {
	T_(begin)(0); T_(exists)(0); T_(look)(0); T_(next)(0);
	T_(head)(0); T_(tail)(0); T_(link_previous)(0); T_(link_next)(0);
	T_(clear)(0); T_(add_before)(0, 0); T_(add_after)(0, 0);
	T_(unshift)(0, 0); T_(push)(0, 0); T_(remove)(0);
	T_(shift)(0); T_(pop)(0); T_(to)(0, 0);
	T_(to_before)(0, 0); T_(self_correct)(0);
#		ifdef HAVE_ITERATE_H
	T_(to_if)(0, 0, 0);
#		endif
	pT_(unused_base_coda)();
}
static void pT_(unused_base_coda)(void) { pT_(unused_base)(); }

#	endif /* Produce code. */
#endif /* base code --> */

#ifndef LIST_DECARE_ONLY /* Produce code. */

#	if defined(LIST_TO_STRING)
#		undef LIST_TO_STRING
/** Type of `LIST_TO_STRING` needed function `<tr>to_string`. Responsible for
 turning the read-only argument into a 12-max-`char` output string. */
typedef void (*pTR_(to_string_fn))(const struct t_(listlink) *, char (*)[12]);
/** Thunk. One must implement `<tr>to_string`. */
static void pTR_(to_string)(const struct T_(cursor) *const cur,
	char (*const a)[12]) { tr_(to_string)(cur->link, a); }
#		include "to_string.h" /** \include */
#		ifndef LIST_TRAIT
#			define LIST_HAS_TO_STRING /* Warning about tests. */
#		endif
#	endif

#	if defined(LIST_TEST) && !defined(LIST_TRAIT)
#		include "../test/test_list.h"
#	endif

#	if (defined(LIST_COMPARE) || defined(LIST_IS_EQUAL))
#		ifdef LIST_COMPARE
#			define COMPARE LIST_COMPARE
#		else
#			define COMPARE_IS_EQUAL LIST_IS_EQUAL
#		endif
#		include "compare.h" /** \include */
#		ifdef LIST_COMPARE /* More functions for ordered lists. */

/** Merges `from` into `to`, preferring elements from `to` go in the front.
 @order \O(|`from`| + |`to`|). @allow */
static void TR_(merge)(struct t_(list) *restrict const to,
	struct t_(list) *restrict const from) {
	struct t_(listlink) *head, **x = &head, *prev = &to->u.as_head.head, *t, *f;
	assert(to && to->u.flat.next && to->u.flat.prev
		&& from && from->u.flat.next && from->u.flat.prev && from != to);
	/* Empty. */
	if(!(f = from->u.flat.next)->next) return;
	if(!(t = to->u.flat.next)->next)
		{ pT_(move)(from, &to->u.as_tail.tail); return; }
	/* Exclude sentinel. */
	from->u.flat.prev->next = to->u.flat.prev->next = 0;
	/* Merge. */
	for( ; ; ) {
		if(t_(compare)(t, f) <= 0) {
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
static void pTR_(merge_runs)(struct t_(listlink) **const head_ptr) {
	struct t_(listlink) *head = *head_ptr, **x = &head, *b = head, *a = b->prev,
		*const prev = a->prev;
	assert(head_ptr && a && b);
	for( ; ; ) {
		if(t_(compare)(a, b) <= 0) {
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
static void pTR_(merge_final)(struct t_(list) *const list,
	struct t_(listlink) *head) {
	struct t_(listlink) *prev = 0, **x = &list->u.flat.next,
		*b = head, *a = head->prev;
	assert(list && b && a && !a->prev);
	for( ; ; ) {
		if(t_(compare)(a, b) <= 0) {
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
static void TR_(sort)(struct t_(list) *const list) {
	/* Add `[-1,0,1]`: unique identifier for nine weakly-ordered transitions. */
	enum { DEC = 1, EQ = 4, INC = 7 };
	int mono = EQ, cmp;
	struct t_(listlink) *a, *b, *c, *dec_iso = /* Unused. */0;
	struct { size_t count; struct t_(listlink) *head, *prev; } run;
	/* Closed sentinel list. */
	assert(list
		&& list->u.flat.next && !list->u.flat.zero && list->u.flat.prev);
	if(a = list->u.flat.next, !(b = a->next)) return; /* Empty. */
	/* Identify runs of monotonicity until `b` sentinel. */
	run.count = 0, run.prev = 0, run.head = a;
	for(c = b->next; c; a = b, b = c, c = c->next) {
		cmp = t_(compare)(b, a);
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
				pTR_(merge_runs)(&run.prev);
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
	while(run.head->prev->prev) pTR_(merge_runs)(&run.head);
	pTR_(merge_final)(list, run.head);
}
/** `alist` `op` `blist` -> `result`. Prefers `a` to `b` when equal. Either
 could be null. @order \O(|`a`| + |`b`|) */
static void pTR_(boolean)(struct t_(list) *restrict const alist,
	struct t_(list) *restrict const blist,
	struct t_(list) *restrict const result, const enum list_operation op) {
	struct t_(listlink) *temp,
		*a = alist ? alist->u.flat.next : 0,
		*b = blist ? blist->u.flat.next : 0;
	int comp;
	/* This is inefficient in the sense that runs will be re-assigned the same
	 pointers as before. Probably doesn't matter because why would you actually
	 use this anyway? */
	assert((!result || (result != alist && result != blist))
		&& (!alist || (alist != blist)));
	if(a && b) {
		while(a->next && b->next) {
			comp = t_(compare)(a, b);
			if(comp < 0) {
				temp = a, a = a->next;
				if(op & LIST_SUBTRACTION_AB) {
					pT_(rm)(temp);
					if(result) pT_(push)(result, temp);
				}
			} else if(comp > 0) {
				temp = b, b = b->next;
				if(op & LIST_SUBTRACTION_BA) {
					pT_(rm)(temp);
					if(result) pT_(push)(result, temp);
				}
			} else {
				temp = a, a = a->next, b = b->next;
				if(op & LIST_INTERSECTION) {
					pT_(rm)(temp);
					if(result) pT_(push)(result, temp);
				}
			}
		}
	}
	if(a && op & LIST_DEFAULT_A) {
		while((temp = a, a = a->next)) {
			pT_(rm)(temp);
			if(result) pT_(push)(result, temp);
		}
	}
	if(b && op & LIST_DEFAULT_B) {
		while((temp = b, b = b->next)) {
			pT_(rm)(temp);
			if(result) pT_(push)(result, temp);
		}
	}
}
/** Subtracts `a` from `b`, as sequential sorted individual elements, and moves
 it to `result`. All elements are removed from `a`. All parameters must be
 unique or can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:A, a:D)` would be moved to `result`.
 @order \O(|`a`| + |`b`|) @allow */
static void TR_(subtraction_to)(struct t_(list) *restrict const a,
	struct t_(list) *restrict const b, struct t_(list) *restrict const result)
	{ pTR_(boolean)(a, b, result, LIST_SUBTRACTION_AB | LIST_DEFAULT_A); }
/** Moves the union of `a` and `b` as sequential sorted individual elements to
 `result`. Equal elements are moved from `a`. All parameters must be unique or
 can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:A, a:B, b:C, a:D)` would be moved to `result`.
 @order \O(|`a`| + |`b`|) @allow */
static void TR_(union_to)(struct t_(list) *restrict const a,
	struct t_(list) *restrict const b, struct t_(list) *restrict const result)
	{ pTR_(boolean)(a, b, result, LIST_SUBTRACTION_AB | LIST_SUBTRACTION_BA
		| LIST_INTERSECTION | LIST_DEFAULT_A | LIST_DEFAULT_B); }
/** Moves the intersection of `a` and `b` as sequential sorted individual
 elements to `result`. Equal elements are moved from `a`. All parameters must
 be unique or can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:B)` would be moved to `result`.
 @order \O(|`a`| + |`b`|) @allow */
static void TR_(intersection_to)(struct t_(list) *restrict const a,
	struct t_(list) *restrict const b, struct t_(list) *restrict const result)
	{ pTR_(boolean)(a, b, result, LIST_INTERSECTION); }
/** Moves `a` exclusive-or `b` as sequential sorted individual elements to
 `result`. Equal elements are moved from `a`. All parameters must be unique or
 can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:A, b:C, a:D)` would be moved to `result`.
 @order O(|`a`| + |`b`|) @allow */
static void TR_(xor_to)(struct t_(list) *restrict const a,
	struct t_(list) *restrict const b, struct t_(list) *restrict const result)
	{ pTR_(boolean)(a, b, result, LIST_SUBTRACTION_AB | LIST_SUBTRACTION_BA
		| LIST_DEFAULT_A | LIST_DEFAULT_B); }

#		endif /* More ordered. */

/** Moves all local-duplicates of `from` to the end of `to`.

 For example, if `from` is `(A, B, B, A)`, it would concatenate the second
 `(B)` to `to` and leave `(A, B, A)` in `from`. If one <fn:<CMP>sort> `from`
 first, `(A, A, B, B)`, the global duplicates will be transferred, `(A, B)`.
 @order \O(|`from`|) @allow */
static void TR_(duplicates_to)(struct t_(list) *restrict const from,
	struct t_(list) *restrict const to) {
	struct t_(listlink) *a = from->u.flat.next, *b, *temp;
	assert(from);
	if(!(b = a->next)) return;
	while(b->next) {
		if(!t_(is_equal)(a, b)) {
			a = b, b = b->next;
		} else {
			temp = b, b = b->next;
			pT_(rm)(temp);
			if(to) pT_(push)(to, temp);
		}
	}
}
static void pTR_(unused_extra_compare_coda)(void);
static void pTR_(unused_extra_compare)(void) {
#		ifdef LIST_COMPARE
	TR_(merge)(0, 0); TR_(sort)(0); TR_(subtraction_to)(0, 0, 0);
	TR_(union_to)(0, 0, 0); TR_(intersection_to)(0, 0, 0);
	TR_(xor_to)(0, 0, 0);
#		endif
	TR_(duplicates_to)(0, 0); pTR_(unused_extra_compare_coda)();
}
static void pTR_(unused_extra_compare_coda)(void){pTR_(unused_extra_compare)();}

#		ifdef LIST_TEST
#			include "../test/test_list_compare.h"
#		endif
#		ifdef LIST_COMPARE
#			undef LIST_COMPARE
#		else
#			undef LIST_IS_EQUAL
#		endif
#	endif

#endif /* Produce code. */
#ifdef LIST_TRAIT
#	undef LIST_TRAIT
#	undef BOX_TRAIT
#endif

/** Thunk `l` -> `a`. */
/*static void PLT_(to_string)(const struct t_(listlink) *l, char (*const a)[12])
	{ LT_(to_string)(l, a); }*/

#ifdef LIST_EXPECT_TRAIT
#	undef LIST_EXPECT_TRAIT
#else
#	undef BOX_MINOR
#	undef BOX_MAJOR
#	undef LIST_NAME
#	ifdef LIST_HAS_TO_STRING
#		undef LIST_HAS_TO_STRING
#	endif
#	ifdef LIST_TEST
#		undef LIST_TEST
#	endif
#	ifdef LIST_DECLARE_ONLY
#		undef LIST_DECLARE_ONLY
#	endif
#endif
#define BOX_END
#include "box.h"
