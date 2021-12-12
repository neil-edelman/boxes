/** @license 2017 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Doubly-linked component

 ![Example of a stochastic skip-list.](../web/list.png)

 In parlance of <Thareja 2014, Data Structures>, <tag:<L>list> is a circular
 header doubly-linked list of <tag:<L>listlink>. The header, or sentinel,
 resides in `<L>list`. This allows it to benefit from being closed structure,
 such that with with a pointer to any element, it is possible to extract the
 entire list in \O(`size`). It only provides an order component, and is not
 very useful without enclosing `<L>listlink` in another `struct`; this is
 useful for multi-linked elements.

 @param[LIST_NAME]
 `<L>` that satisfies `C` naming conventions when mangled; required. `<PL>` is
 private, whose names are prefixed in a manner to avoid collisions.

 @param[LIST_COMPARE]
 Optional total-order function satisfying <typedef:<PL>compare_fn>.
 (fixme: move to trait.)

 @param[LIST_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a trait.

 @param[LIST_TO_STRING_NAME, LIST_TO_STRING]
 To string trait contained in <to_string.h>; requires `ARRAY_ITERATE` and goes
 forwards. An optional mangled name for uniqueness and function implementing
 <typedef:<PSZ>to_string_fn>.

 @param[LIST_TEST]
 To string trait contained in <../test/test_list.h>; optional unit testing
 framework using `assert`. Can only be defined once _per_ `Array`. Must be
 defined equal to a (random) filler function, satisfying
 <typedef:<PL>action_fn>. Output will be shown with the to string trait in
 which it's defined; provides tests for the base code and all later traits.

 @std C89 */

#ifndef LIST_NAME
#error Name LIST_NAME undefined.
#endif
#if defined(LIST_TO_STRING_NAME) || defined(LIST_TO_STRING) /* <!-- str */
#define LIST_TO_STRING_TRAIT 1
#else /* str --><!-- !str */
#define LIST_TO_STRING_TRAIT 0
#endif /* !str --> */
#define LIST_TRAITS LIST_TO_STRING_TRAIT
#if LIST_TRAITS > 1
#error Only one trait per include is allowed; use LIST_EXPECT_TRAIT.
#endif
#if defined(LIST_TO_STRING_NAME) && !defined(LIST_TO_STRING)
#error LIST_TO_STRING_NAME requires LIST_TO_STRING.
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
/* <fn:<PL>boolean> operations bit-vector; dummy `LO_` ensures closed. */
enum list_operation {
	LO_SUBTRACTION_AB = 1,
	LO_SUBTRACTION_BA = 2,
	LO_A,
	LO_INTERSECTION   = 4,
	LO_B, LO_C, LO_D,
	LO_DEFAULT_A      = 8,
	LO_E, LO_F, LO_G, LO_H, LO_I, LO_J, LO_K,
	LO_DEFAULT_B      = 16,
	LO_L, LO_M, LO_N, LO_O, LO_P, LO_Q, LO_R, LO_S,
	LO_T, LO_U, LO_V, LO_W, LO_X, LO_Y, LO_Z
};
#endif /* idempotent --> */


#if LIST_TRAITS == 0 /* <!-- base code */


/* A note about <tag:<L>listlink> and <tag:<L>list>: these don't have to be
 parameterized at all. However, it's more type-safe to have separate types if
 we are coercing them. */

/* ************* FIXME: update the images; they are from a version 10 years ago.
 ***********/
/* ********** FIXME: have an option to throw an error if a link is not null
 zero() clears and zeros, push(), add() makes sure it's zero,
 no, have an option **********/
/** Storage of this structure is the responsibility of the caller. Generally,
 one encloses this in a host `struct`. Multiple independent lists can be in the
 same host structure, however one link can can only be a part of one list at a
 time; adding a link to a second list destroys the integrity of the original
 list.

 ![States.](../web/node-states.png) */
struct L_(listlink) { struct L_(listlink) *prev, *next; };

/** Serves as head and tail for linked-list of <tag:<L>listlink>. Use
 <fn:<L>list_clear> to initialize the list. Because this list is closed; that
 is, given a valid pointer to an element, one can determine all others, null
 values are not allowed and it is _not_ the same as `{0}`. These are sentinels
 such that `head.prev` and `tail.next` are always and the only ones to be null
 in a valid list.

 ![States.](../web/states.png) */
struct L_(list) { struct L_(listlink) head, tail; };

/* *******FIXME move to trait*********/

/** Operates by side-effects on the node. */
typedef void (*PL_(action_fn))(struct L_(listlink) *);

/** Returns (Non-zero) true or (zero) false when given a node. */
typedef int (*PL_(predicate_fn))(const struct L_(listlink) *);

/** Returns less then, equal to, or greater then zero, inducing an ordering
 between `a` and `b`. */
typedef int (*PL_(compare_fn))(const struct L_(listlink) *a,
	const struct L_(listlink) *b);

/** Cats all `from` in front of `node`, (don't make `node` `head`); `from` will
 be empty after. Careful that `node` is not in `from` because that will just
 erase the list. @order \Theta(1) */
static void PL_(move)(struct L_(list) *const from,
	struct L_(listlink) *const node) {
	assert(node && from && node->prev &&
		!from->head.prev && from->head.next
		&& from->tail.prev && !from->tail.next);
	from->head.next->prev = node->prev;
	node->prev->next = from->head.next;
	from->tail.prev->next = node;
	node->prev = from->tail.prev;
	from->head.next = &from->tail;
	from->tail.prev = &from->head;
}

/** @return A pointer to the first element of `list`, if it exists.
 @order \Theta(1) @allow */
static struct L_(listlink) *L_(list_first)(const struct L_(list) *const list) {
	struct L_(listlink) *link;
	assert(list);
	link = list->head.next, assert(link);
	return link->next ? link : 0;
}

/** @return A pointer to the last element of `list`, if it exists.
 @order \Theta(1) @allow */
static struct L_(listlink) *L_(list_last)(const struct L_(list) *const list) {
	struct L_(listlink) *link;
	assert(list);
	link = list->tail.prev, assert(link);
	return link->prev ? link : 0;
}

/** @return The previous element. When `link` is the first element, returns
 null. @order \Theta(1) @allow */
static struct L_(listlink) *L_(list_previous)(struct L_(listlink) *link) {
	assert(link && link->prev);
	link = link->prev;
	return link->prev ? link : 0;
}

/** @return The next element. When `link` is the last element, returns null.
 @order \Theta(1) @allow */
static struct L_(listlink) *L_(list_next)(struct L_(listlink) *link) {
	assert(link && link->next);
	link = link->next;
	return link->next ? link : 0;
}

/** Clears and initializes `list`.  @order \Theta(1) @allow */
static void L_(list_clear)(struct L_(list) *const list) {
	assert(list);
	list->head.prev = list->tail.next = 0;
	list->head.next = &list->tail;
	list->tail.prev = &list->head;
}

/** `add` before `anchor`. @order \Theta(1) @allow */
static void L_(list_add_before)(struct L_(listlink) *const anchor,
	struct L_(listlink) *const add) {
	assert(anchor && add && anchor != add && anchor->prev);
	add->prev = anchor->prev;
	add->next = anchor;
	anchor->prev->next = add;
	anchor->prev = add;
}

/** `add` after `anchor`. @order \Theta(1) @allow */
static void L_(list_add_after)(struct L_(listlink) *const anchor,
	struct L_(listlink) *const add) {
	assert(anchor && add && anchor != add && anchor->next);
	add->prev = anchor;
	add->next = anchor->next;
	anchor->next->prev = add;
	anchor->next = add;
}

/** Adds `add` to the beginning of `list`. @order \Theta(1) @allow */
static void L_(list_unshift)(struct L_(list) *const list,
	struct L_(listlink) *const add)
	{ assert(list && add), L_(list_add_after)(&list->head, add); }

/** Adds `add` to the end of `list`. @order \Theta(1) @allow */
static void L_(list_push)(struct L_(list) *const list,
	struct L_(listlink) *const add)
	{ assert(list && add), L_(list_add_before)(&list->tail, add); }

/** Remove `node`. @order \Theta(1) @allow */
static void L_(list_remove)(struct L_(listlink) *const node) {
	assert(node && node->prev && node->next);
	node->prev->next = node->next;
	node->next->prev = node->prev;
	node->prev = node->next = 0;
}

/** Removes the first element of `list` and returns it, if any.
 @order \Theta(1) @allow */
static struct L_(listlink) *L_(list_shift)(struct L_(list) *const list) {
	struct L_(listlink) *node;
	assert(list && list->head.next);
	if(!(node = list->head.next)->next) return 0;
	L_(list_remove)(node);
	return node;
}

/** Removes the last element of `list` and returns it, if any.
 @order \Theta(1) @allow */
static struct L_(listlink) *L_(list_pop)(struct L_(list) *const list) {
	struct L_(listlink) *node;
	assert(list && list->tail.prev);
	if(!(node = list->tail.prev)->prev) return 0;
	L_(list_remove)(node);
	return node;
}

/** Moves the elements `from` onto `to` at the end.
 @param[to] If null, then it removes elements from `from`.
 @order \Theta(1) @allow */
static void L_(list_to)(struct L_(list) *const from,
	struct L_(list) *const to) {
	assert(from && from != to);
	if(!to) { L_(list_clear)(from); return; }
	PL_(move)(from, &to->tail);
}

/** Moves the elements `from` immediately before `anchor`.
 @order \Theta(1) @allow */
static void L_(list_to_before)(struct L_(list) *const from,
	struct L_(listlink) *const anchor) {
	assert(from && anchor);
	PL_(move)(from, anchor);
}

/** Moves all elements `from` onto `to` at the end if `predicate` is true.
 @param[to] If null, then it removes elements.
 @order \Theta(|`from`|) \times \O(`predicate`) @allow */
static void L_(list_to_if)(struct L_(list) *const from,
	struct L_(list) *const to, const PL_(predicate_fn) predicate) {
	struct L_(listlink) *link, *next_link;
	assert(from && from != to && predicate);
	for(link = from->head.next; (next_link = link->next); link = next_link) {
		if(!predicate(link)) continue;
		L_(list_remove)(link);
		if(to) L_(list_add_before)(&to->tail, link);
	}
}

/** Performs `action` for each element in `list` in order.
 @param[action] Can be to delete the element.
 @order \Theta(|`list`|) \times O(`action`) @allow */
static void L_(list_for_each)(struct L_(list) *const list,
	const PL_(action_fn) action) {
	struct L_(listlink) *x, *next_x;
	assert(list && action);
	for(x = list->head.next; (next_x = x->next); x = next_x)
		action(x);
}

/** Iterates through `list` and calls `predicate` until it returns true.
 @return The first `predicate` that returned true, or, if the statement is
 false on all, null.
 @order \O(|`list`|) \times \O(`predicate`) @allow */
static struct L_(listlink) *L_(list_any)(const struct L_(list) *const list,
	const PL_(predicate_fn) predicate) {
	struct L_(listlink) *link, *next_link;
	assert(list && predicate);
	for(link = list->head.next; (next_link = link->next); link = next_link)
		if(predicate(link)) return link;
	return 0;
}

/** Corrects `list` ends to compensate for memory relocation of the list
 itself. @order \Theta(1) @allow */
static void L_(list_self_correct)(struct L_(list) *const list) {
	assert(list);
	if(list->head.next == list->tail.prev + 1) {
		list->head.next = &list->tail;
		list->tail.prev = &list->head;
	} else {
		list->head.next->prev = &list->head;
		list->tail.prev->next = &list->tail;
	}
}

#ifdef LIST_COMPARE /* <!-- comp: fixme: move all this to compare.h. */


/* Check that `LIST_COMPARE` is a function implementing
 <typedef:<PL>compare_fn>. */
static const PL_(compare_fn) PL_(compare) = (LIST_COMPARE);

/** Private: `alist` `mask` `blist` -> `result`. Prefers `a` to `b` when equal.
 @order \O(|`a`| + |`b`|) */
static void PL_(boolean)(struct L_(list) *const alist,
	struct L_(list) *const blist, const enum list_operation mask,
	struct L_(list) *const result) {
	struct L_(listlink) *a = alist ? alist->head.next : 0,
		*b = blist ? blist->head.next : 0, *temp;
	int comp;
	assert((!result || (result != alist && result != blist))
		&& (!alist || (alist != blist)));
	if(a && b) {
		while(a->next && b->next) {
			comp = PL_(compare)(a, b);
			if(comp < 0) {
				temp = a, a = a->next;
				if(mask & LO_SUBTRACTION_AB) {
					L_(list_remove)(temp);
					if(result) L_(list_add_before)(&result->tail, temp);
				}
			} else if(comp > 0) {
				temp = b, b = b->next;
				if(mask & LO_SUBTRACTION_BA) {
					L_(list_remove)(temp);
					if(result) L_(list_add_before)(&result->tail, temp);
				}
			} else {
				temp = a, a = a->next, b = b->next;
				if(mask & LO_INTERSECTION) {
					L_(list_remove)(temp);
					if(result) L_(list_add_before)(&result->tail, temp);
				}
			}
		}
	}
	if(a && mask & LO_DEFAULT_A) {
		while((temp = a, a = a->next)) {
			L_(list_remove)(temp);
			if(result) L_(list_add_before)(&result->tail, temp);
		}
	}
	if(b && mask & LO_DEFAULT_B) {
		while((temp = b, b = b->next)) {
			L_(list_remove)(temp);
			if(result) L_(list_add_before)(&result->tail, temp);
		}
	}
}

/** Used in <fn:<PL>sort>: merges the two top runs referenced by `head_ptr` in
 stack form. */
static void PL_(merge_runs)(struct L_(listlink) **const head_ptr) {
	struct L_(listlink) *head = *head_ptr, **x = &head,
		*b = head, *a = b->prev, *const prev = a->prev;
	assert(head_ptr && a && b);
	for( ; ; ) {
		if(PL_(compare)(a, b) <= 0) {
			*x = a, x = &a->next;
			if(!(a = a->next)) { *x = b; break; }
		} else {
			*x = b, x = &b->next;
			if(!(b = b->next)) { *x = a; break; }
		}
	}
	head->prev = prev, *head_ptr = head;
}

/** The list form of `list` is restored from `head` in stack form with two
 runs. */
static void PL_(merge_final)(struct L_(list) *const list,
	struct L_(listlink) *head) {
	struct L_(listlink) *prev = 0, **x = &list->head.next,
		*b = head, *a = head->prev;
	assert(list && !list->head.prev && !list->tail.next && b && a && !a->prev);
	for( ; ; ) {
		if(PL_(compare)(a, b) <= 0) {
			a->prev = prev, prev = *x = a, x = &a->next;
			if(!(a = a->next)) { a = *x = b; break; }
		} else {
			b->prev = prev, prev = *x = b, x = &b->next;
			if(!(b = b->next)) { *x = a; break; }
		}
	}
	do; while(a->prev = prev, prev = a, a = a->next);
	prev->next = &list->tail, list->tail.prev = prev;
	assert(list->head.next && list->head.next != &list->tail); /* Not empty. */
	list->head.next->prev = &list->head;
}

/** Natural merge sort `list`; the requirement for \O(\log |`list`|) space is
 satisfied by converting it to a singly-linked with `prev` as a stack of
 increasing lists, which are merged. */
static void PL_(sort)(struct L_(list) *const list) {
	/* Add `[-1,0,1]`: unique identifier for nine weakly-ordered transitions. */
	enum { DEC = 1, EQ = 4, INC = 7 };
	int mono = EQ, cmp;
	struct L_(listlink) *a, *b, *c, *dec_iso = /* Unused. */0;
	struct { size_t count; struct L_(listlink) *head, *prev; } run;
	/* Closed sentinel list. */
	assert(list && list->head.next && list->tail.prev
		&& !list->head.prev && !list->tail.next);
	if(a = list->head.next, !(b = a->next)) return; /* Empty. */
	/* Identify runs of monotonicity until `b` sentinel. */
	run.count = 0, run.prev = 0, run.head = a;
	for(c = b->next; c; a = b, b = c, c = c->next) {
		cmp = PL_(compare)(b, a);
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
				PL_(merge_runs)(&run.prev);
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
			list->head.next = dec_iso, dec_iso->prev = &list->head;
			for(a = dec_iso, b = a->next; b; a = b, b = b->next) b->prev = a;
			list->tail.prev = a, a->next = &list->tail;
			return; /* Reverse sorted; now good as well. */
		}
	}
	assert(run.count);
	/* (Actually slower to merge the last one eagerly. So do nothing.) */
	run.head->prev = run.prev, run.count++;
	/* Merge leftovers from the other direction, saving one for final. */
	while(run.head->prev->prev) PL_(merge_runs)(&run.head);
	PL_(merge_final)(list, run.head);
}

/** Performs a stable, adaptive sort of `list` according to `compare`. Requires
 `LIST_COMPARE`. Sorting a list is always going to be slower then sorting an
 array for some number of items.
 @order \Omega(|`list`|), \O(|`list`| log |`list`|) @allow */
static void L_(list_sort)(struct L_(list) *const list) { PL_(sort)(list); }

/** Merges from `from` into `to`. If the elements are sorted in both lists,
 then the elements of `list` will be sorted.
 @order \O(|`from`| + |`to`|). */
static void L_(list_merge)(struct L_(list) *const from,
	struct L_(list) *const to) {
	struct L_(listlink) *cur, *a, *b;
	assert(from && from->head.next && to && to->head.next && from != to);
	/* `blist` empty -- that was easy. */
	if(!(b = from->head.next)->next) return;
	/* `alist` empty -- `O(1)` <fn:<PL>move> is more efficient. */
	if(!(a = to->head.next)->next)
	{ PL_(move)(from, &to->tail); return; }
	/* Merge */
	for(cur = &to->head; ; ) {
		if(PL_(compare)(a, b) < 0) {
			a->prev = cur, cur = cur->next = a;
			if(!(a = a->next)->next) {
				b->prev = cur, cur->next = b;
				from->tail.prev->next = &to->tail;
				to->tail.prev = from->tail.prev;
				break;
			}
		} else {
			b->prev = cur, cur = cur->next = b;
			if(!(b = b->next)->next) { a->prev = cur, cur->next = a; break; }
		}
	}
	from->head.next = &from->tail, from->tail.prev = &from->head;
}

/** Compares `alist` to `blist` as sequences.
 @return The first `LIST_COMPARE` that is not equal to zero, or 0 if they are
 equal. Null is considered as before everything else; two null pointers are
 considered equal. @implements <typedef:<PL>compare_fn>
 @order \Theta(min(|`alist`|, |`blist`|)) @allow */
static int L_(list_compare)(const struct L_(list) *const alist,
	const struct L_(list) *const blist) {
	struct L_(listlink) *a, *b;
	int diff;
	/* Null counts as `-\infty`. */
	if(!alist) {
		return blist ? -1 : 0;
	} else if(!blist) {
		return 1;
	}
	/* Compare element by element. */
	for(a = alist->head.next, b = blist->head.next; ; a = a->next, b = b->next)
	{
		if(!a->next) {
			return b->next ? -1 : 0;
		} else if(!b->next) {
			return 1;
		} else if((diff = PL_(compare)(a, b))) {
			return diff;
		}
	}
}

/** Moves all local-duplicates of `from` to the end of `to`.

 For example, if `from` is `(A, B, B, A)`, it would concatenate `(B)` to `to`
 and leave `(A, B, A)` in `from`. If one <fn:<L>list_sort> `from` first,
 `(A, A, B, B)`, the global duplicates will be transferred, `(A, B)`.
 @order \O(|`from`|) @allow */
static void L_(list_duplicates_to)(struct L_(list) *const from,
	struct L_(list) *const to) {
	struct L_(listlink) *a = from->head.next, *b, *temp;
	assert(from);
	if(!(b = a->next)) return;
	while(b->next) {
		if(PL_(compare)(a, b)) {
			a = b, b = b->next;
		} else {
			temp = b, b = b->next;
			L_(list_remove)(temp);
			if(to) L_(list_add_before)(&to->tail, temp);
		}
	}
}

/** Subtracts `a` from `b`, as sequential sorted individual elements, and moves
 it to `result`. All elements are removed from `a`. All parameters must be
 unique or can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:A, a:D)` would be moved to `result`.
 @order \O(|`a`| + |`b`|) @allow */
static void L_(list_subtraction_to)(struct L_(list) *const a,
	struct L_(list) *const b, struct L_(list) *const result) {
	PL_(boolean)(a, b, LO_SUBTRACTION_AB | LO_DEFAULT_A, result);
}

/** Moves the union of `a` and `b` as sequential sorted individual elements to
 `result`. Equal elements are moved from `a`. All parameters must be unique or
 can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:A, a:B, b:C, a:D)` would be moved to `result`.
 @order \O(|`a`| + |`b`|) @allow */
static void L_(list_union_to)(struct L_(list) *const a,
	struct L_(list) *const b, struct L_(list) *const result) {
	PL_(boolean)(a, b, LO_SUBTRACTION_AB | LO_SUBTRACTION_BA | LO_INTERSECTION
		| LO_DEFAULT_A | LO_DEFAULT_B, result);
}

/** Moves the intersection of `a` and `b` as sequential sorted individual
 elements to `result`. Equal elements are moved from `a`. All parameters must
 be unique or can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:B)` would be moved to `result`.
 @order \O(|`a`| + |`b`|) @allow */
static void L_(list_intersection_to)(struct L_(list) *const a,
	struct L_(list) *const b, struct L_(list) *const result) {
	PL_(boolean)(a, b, LO_INTERSECTION, result);
}

/** Moves `a` exclusive-or `b` as sequential sorted individual elements to
 `result`. Equal elements are moved from `a`. All parameters must be unique or
 can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:A, b:C, a:D)` would be moved to `result`.
 @order O(|`a`| + |`b`|) @allow */
static void L_(list_xor_to)(struct L_(list) *const a, struct L_(list) *const b,
	struct L_(list) *const result) {
	PL_(boolean)(a, b, LO_SUBTRACTION_AB | LO_SUBTRACTION_BA | LO_DEFAULT_A
		| LO_DEFAULT_B, result);
}

#endif /* comp --> */

/* <!-- iterate interface */

/********** FIXME: don't duplicate; this should be the private implementation
 of the functions above <fn:<L>list_next>, _etc_. ***********/

/** Contains all iteration parameters. */
struct PL_(iterator) { struct L_(listlink) *node; };

/** Loads `list` into `it`. @implements begin */
static void PL_(begin)(struct PL_(iterator) *const it,
	const struct L_(list) *const list)
	{ assert(it && list), it->node = list->head.next /*L_(list_first)(list)*/; }

/** Advances `it`. @implements next */
static const struct L_(listlink) *PL_(next)(struct PL_(iterator) *const it) {
	struct L_(listlink) *n;
	return assert(it && it->node), (it->node = (n = it->node)->next) ? n : 0;
	/* it->node = L_(list_next)(it->node) */
}

/* iterate --><!-- reverse interface */

/** Loads `list` into `it`. @implements begin */
static void PL_(end)(struct PL_(iterator) *const it,
	const struct L_(list) *const list)
	{ assert(it && list), it->node = list->tail.prev; }

/** Advances `it`. @implements next */
static const struct L_(listlink) *PL_(previous)(struct PL_(iterator) *const it)
{
	struct L_(listlink) *n;
	return assert(it && it->node), (it->node = (n = it->node)->prev) ? n : 0;
}

/* reverse --> */

/* <!-- box (multiple traits) */
#define BOX_ PL_
#define BOX_CONTAINER struct L_(list)
#define BOX_CONTENTS struct L_(listlink)

#ifdef LIST_TEST /* <!-- test */
/* Forward-declare. */
static void (*PL_(to_string))(const struct L_(listlink) *, char (*)[12]);
static const char *(*PL_(list_to_string))(const struct L_(list) *);
#include "../test/test_list.h" /* (no) \include */
#endif /* test --> */

static void PL_(unused_base_coda)(void);
static void PL_(unused_base)(void) {
	L_(list_first)(0); L_(list_last)(0); L_(list_previous)(0); L_(list_next)(0);
	L_(list_clear)(0); L_(list_add_before)(0, 0); L_(list_add_after)(0, 0);
	L_(list_unshift)(0, 0); L_(list_push)(0, 0); L_(list_remove)(0);
	L_(list_shift)(0); L_(list_pop)(0); L_(list_to)(0, 0);
	L_(list_to_before)(0, 0); L_(list_to_if)(0, 0, 0); L_(list_for_each)(0, 0);
	L_(list_any)(0, 0); L_(list_self_correct)(0);
#ifdef LIST_COMPARE /* <!-- comp */
	L_(list_sort)(0); L_(list_merge)(0, 0); L_(list_compare)(0, 0);
	L_(list_duplicates_to)(0, 0); L_(list_subtraction_to)(0, 0, 0);
	L_(list_union_to)(0, 0, 0); L_(list_intersection_to)(0, 0, 0);
	L_(list_xor_to)(0, 0, 0);
#endif /* comp --> */
	PL_(begin)(0, 0); PL_(next)(0); PL_(end)(0, 0); PL_(previous)(0);
	PL_(unused_base_coda)();
}
static void PL_(unused_base_coda)(void) { PL_(unused_base)(); }


#elif defined(LIST_TO_STRING) /* base code --><!-- to string trait */


#ifdef LIST_TO_STRING_NAME /* <!-- name */
#define SZ_(n) LIST_CAT(L_(list), LIST_CAT(LIST_TO_STRING_NAME, n))
#else /* name --><!-- !name */
#define SZ_(n) LIST_CAT(L_(list), n)
#endif /* !name --> */
#define TO_STRING LIST_TO_STRING
#include "to_string.h" /** \include */
#ifdef LIST_TEST /* <!-- expect: greedy satisfy forward-declared. */
#undef LIST_TEST
static PSZ_(to_string_fn) PL_(to_string) = PSZ_(to_string);
static const char *(*PL_(list_to_string))(const struct L_(list) *)
	= &SZ_(to_string);
#endif /* expect --> */
#undef SZ_
#undef LIST_TO_STRING
#ifdef LIST_TO_STRING_NAME
#undef LIST_TO_STRING_NAME
#endif


#endif /* traits --> */


#ifdef LIST_EXPECT_TRAIT /* <!-- trait */
#undef LIST_EXPECT_TRAIT
#else /* trait --><!-- !trait */
#if defined(LIST_TEST)
#error No to string traits defined for test.
#endif
#undef LIST_NAME
#ifdef LIST_COMPARE
#undef LIST_COMPARE
#endif
#undef BOX_
#undef BOX_CONTAINER
#undef BOX_CONTENTS
#endif /* !trait --> */
#undef LIST_TO_STRING_TRAIT
#undef LIST_TRAITS
