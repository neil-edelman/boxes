/** @license 2017 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Doubly-linked component

 ![Example of a stochastic skip-list.](../web/list.png)

 In parlance of <Thareja 2014, Structures>, <tag:<L>list> is a circular
 header, or sentinel, to a doubly-linked list of <tag:<L>listlink>. This allows
 it to benefit from being closed structure, such that with with a pointer to
 any element, it is possible to extract the entire list.

 @param[LIST_NAME]
 `<L>` that satisfies `C` naming conventions when mangled; required. `<PL>` is
 private, whose names are prefixed in a manner to avoid collisions.

 @param[LIST_COMPARE]
 Optional total-order function satisfying <typedef:<PL>compare_fn>.
 (fixme: move to trait; wait, I thought it was already?)

 @param[LIST_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a trait.

 @param[LIST_TO_STRING_NAME, LIST_TO_STRING]
 To string trait contained in <to_string.h>. An optional mangled name for
 uniqueness and function implementing <typedef:<PSZ>to_string_fn>.

 @std C89 */

#ifndef LIST_NAME
#error Name LIST_NAME undefined.
#endif
#if defined(LIST_TO_STRING_NAME) || defined(LIST_TO_STRING) /* <!-- str */
#define LIST_TO_STRING_TRAIT 1
#else /* str --><!-- !str */
#define LIST_TO_STRING_TRAIT 0
#endif /* !str --> */
#if defined(LIST_COMPARE) || defined(LIST_COMPARE_NAME) /* <!-- comp */
#define LIST_COMPARE_TRAIT 1
#else /* comp --><!-- !comp */
#define LIST_COMPARE_TRAIT 0
#endif /* !comp --> */
#define LIST_TRAITS LIST_TO_STRING_TRAIT + LIST_COMPARE_TRAIT
#if LIST_TRAITS > 1
#error Only one trait per include is allowed; use LIST_EXPECT_TRAIT.
#endif
#if defined(LIST_TO_STRING_NAME) && !defined(LIST_TO_STRING)
#error LIST_TO_STRING_NAME requires LIST_TO_STRING.
#endif
#if defined(LIST_COMPARE_NAME) && !defined(LIST_COMPARE)
#error LIST_COMPARE_NAME requires LIST_COMPARE.
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
/* <fn:<PL>boolean> operations bit-vector; dummy `LIST_` ensures closed. */
enum list_operation {
	LIST_SUBTRACTION_AB = 1,
	LIST_SUBTRACTION_BA = 2,  LISTA,
	LIST_INTERSECTION   = 4,  LISTB, LISTC, LISTD,
	LIST_DEFAULT_A      = 8,  LISTE, LISTF, LISTG, LISTH, LISTI, LISTJ, LISTK,
	LIST_DEFAULT_B      = 16, LISTL, LISTM, LISTN, LISTO, LISTP, LISTQ, LISTR,
	LISTS, LISTT, LISTU, LISTV, LISTW, LISTX, LISTY, LISTZ
};
#endif /* idempotent --> */


#if LIST_TRAITS == 0 /* <!-- base code */


/** Storage of this structure is the responsibility of the caller. Generally,
 one encloses this in a host `struct` or `union`. Multiple independent lists
 can be in the same host structure, however one link can can only be a part of
 one list at a time; adding a link to a second list destroys the integrity of
 the original list.

 ![States.](../web/node-states.png) */
struct L_(listlink) { struct L_(listlink) *next, *prev; };

/** Serves as head and tail for linked-list of <tag:<L>listlink>. Use
 <fn:<L>list_clear> to initialize the list. Because this list is closed; that
 is, given a valid pointer to an element, one can determine all others, null
 values are not allowed and it is _not_ the same as `{0}`. These are sentinels
 such that `head.prev` and `tail.next` are always and the only ones to be null
 in a valid list.

 ![States.](../web/states.png) */
struct L_(list) {
	union {
		struct { struct L_(listlink) head, *do_not_use; } as_head;
		struct { struct L_(listlink) *do_not_use, tail; } as_tail;
		struct { struct L_(listlink) *next, *zero, *prev; } flat;
	} u;
};

/** Operates by side-effects on the node. */
typedef void (*PL_(action_fn))(struct L_(listlink) *);

/** Returns (Non-zero) true or (zero) false when given a node. */
typedef int (*PL_(predicate_fn))(const struct L_(listlink) *);

/** Clear `list`. */
static void PL_(clear)(struct L_(list) *const list) {
	list->u.flat.next = &list->u.as_tail.tail;
	list->u.flat.zero = 0;
	list->u.flat.prev = &list->u.as_head.head;
}

/** Cats all `from` in front of `after`; `from` will be empty after.
 Careful that `after` is not in `from` because that will just erase the list.
 @order \Theta(1) */
static void PL_(move)(struct L_(list) *const from,
	struct L_(listlink) *const after) {
	assert(from && from->u.flat.next && !from->u.flat.zero && from->u.flat.prev
		&& after && after->prev);
	from->u.flat.next->prev = after->prev;
	after->prev->next = from->u.as_head.head.next;
	from->u.flat.prev->next = after;
	after->prev = from->u.as_tail.tail.prev;
	from->u.flat.next = &from->u.as_tail.tail;
	from->u.flat.prev = &from->u.as_head.head;
}

/** @return A pointer to the first element of `list`, if it exists.
 @order \Theta(1) @allow */
static struct L_(listlink) *L_(list_head)(const struct L_(list) *const list) {
	struct L_(listlink) *link;
	assert(list);
	link = list->u.flat.next, assert(link);
	return link->next ? link : 0;
}

/** @return A pointer to the last element of `list`, if it exists.
 @order \Theta(1) @allow */
static struct L_(listlink) *L_(list_tail)(const struct L_(list) *const list) {
	struct L_(listlink) *link;
	assert(list);
	link = list->u.flat.prev, assert(link);
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
static void L_(list_clear)(struct L_(list) *const list)
	{ assert(list); PL_(clear)(list); }

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
	{ assert(list && add), L_(list_add_after)(&list->u.as_head.head, add); }

/** Adds `add` to the end of `list`. @order \Theta(1) @allow */
static void L_(list_push)(struct L_(list) *const list,
	struct L_(listlink) *const add)
	{ assert(list && add), L_(list_add_before)(&list->u.as_tail.tail, add); }

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
static void L_(list_to)(struct L_(list) *const from,
	struct L_(list) *const to) {
	assert(from && from != to);
	if(!to) { PL_(clear)(from); return; }
	PL_(move)(from, &to->u.as_tail.tail);
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
	for(link = from->u.flat.next; next_link = link->next; link = next_link) {
		if(!predicate(link)) continue;
		L_(list_remove)(link);
		if(to) L_(list_add_before)(&to->u.as_tail.tail, link);
	}
}

/** Performs `action` for each element in `list` in order.
 @param[action] Can be to delete the element.
 @order \Theta(|`list`|) \times O(`action`) @allow */
static void L_(list_for_each)(struct L_(list) *const list,
	const PL_(action_fn) action) {
	struct L_(listlink) *x, *next_x;
	assert(list && action);
	for(x = list->u.flat.next; next_x = x->next; x = next_x)
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
	for(link = list->u.flat.next; next_link = link->next; link = next_link)
		if(predicate(link)) return link;
	return 0;
}

/** Corrects `list` ends to compensate for memory relocation of the list
 itself. Because the `list` is part of the links, this will invalidate all
 other copies. @order \Theta(1) @allow */
static void L_(list_self_correct)(struct L_(list) *const list) {
	assert(list && !list->u.flat.zero);
	if(list->u.flat.next + 2 == list->u.flat.prev) { /* Empty. */
		list->u.flat.next = &list->u.as_tail.tail;
		list->u.flat.prev = &list->u.as_head.head;
	} else { /* Non-empty. */
		list->u.flat.next->prev = &list->u.as_tail.tail;
		list->u.flat.prev->next = &list->u.as_head.head;
	}
}

#ifdef LIST_COMPARE /* <!-- comp: fixme: move all this to compare.h? */

/** Returns less then, equal to, or greater then zero, inducing an ordering
 between `a` and `b`. */
typedef int (*PL_(compare_fn))(const struct L_(listlink) *a,
	const struct L_(listlink) *b);

/* Check that `LIST_COMPARE` is a function implementing
 <typedef:<PL>compare_fn>. */
static const PL_(compare_fn) PL_(compare) = (LIST_COMPARE);

/** Private: `alist` `mask` `blist` -> `result`. Prefers `a` to `b` when equal.
 @order \O(|`a`| + |`b`|) */
static void PL_(boolean)(struct L_(list) *const alist,
	struct L_(list) *const blist, const enum list_operation mask,
	struct L_(list) *const result) {
	struct L_(listlink) *a = alist ? alist->u.flat.next : 0,
		*b = blist ? blist->u.flat.next : 0, *temp;
	int comp;
	assert((!result || (result != alist && result != blist))
		&& (!alist || (alist != blist)));
	if(a && b) {
		while(a->next && b->next) {
			comp = PL_(compare)(a, b);
			if(comp < 0) {
				temp = a, a = a->next;
				if(mask & LIST_SUBTRACTION_AB) {
					L_(list_remove)(temp);
					if(result) L_(list_push)(result, temp);
				}
			} else if(comp > 0) {
				temp = b, b = b->next;
				if(mask & LIST_SUBTRACTION_BA) {
					L_(list_remove)(temp);
					if(result) L_(list_push)(result, temp);
				}
			} else {
				temp = a, a = a->next, b = b->next;
				if(mask & LIST_INTERSECTION) {
					L_(list_remove)(temp);
					if(result) L_(list_push)(result, temp);
				}
			}
		}
	}
	if(a && mask & LIST_DEFAULT_A) {
		while((temp = a, a = a->next)) {
			L_(list_remove)(temp);
			if(result) L_(list_push)(result, temp);
		}
	}
	if(b && mask & LIST_DEFAULT_B) {
		while((temp = b, b = b->next)) {
			L_(list_remove)(temp);
			if(result) L_(list_push)(result, temp);
		}
	}
}

/** Merges the two top runs referenced by `head_ptr` in stack form. */
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
	struct L_(listlink) *prev = 0, **x = &list->u.flat.next,
		*b = head, *a = head->prev;
	assert(list && b && a && !a->prev);
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
	prev->next = &list->u.as_tail.tail, list->u.flat.prev = prev;
	/* Not empty. */
	assert(list->u.flat.next && list->u.flat.next != &list->u.as_tail.tail);
	list->u.flat.next->prev = &list->u.as_head.head;
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
	assert(list
		&& list->u.flat.next && !list->u.flat.zero && list->u.flat.prev);
	if(a = list->u.flat.next, !(b = a->next)) return; /* Empty. */
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
 @order \O(|`from`| + |`to`|). @fixme */
static void L_(list_merge)(struct L_(list) *const from,
	struct L_(list) *const to) {
	struct L_(listlink) *cur, *a, *b;
	assert(from && from->u.flat.next && to && to->u.flat.next && from != to);
	/* `blist` empty -- that was easy. */
	if(!(b = from->u.flat.next)->next) return;
	/* `alist` empty -- `O(1)` <fn:<PL>move> is more efficient. */
	if(!(a = to->u.flat.next)->next)
	{ PL_(move)(from, &to->u.as_tail.tail); return; }
	/* Merge */
	for(cur = &to->u.as_head.head; ; ) {
		if(PL_(compare)(a, b) < 0) {
			a->prev = cur, cur = cur->next = a;
			if(!(a = a->next)->next) {
				b->prev = cur, cur->next = b;
				from->u.flat.prev->next = &to->u.as_tail.tail;
				to->u.flat.prev = from->u.flat.prev;
				break;
			}
		} else {
			b->prev = cur, cur = cur->next = b;
			if(!(b = b->next)->next) { a->prev = cur, cur->next = a; break; }
		}
	}
	from->u.flat.next = &from->u.as_tail.tail;
	from->u.flat.prev = &from->u.as_head.head;
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
	for(a = alist->u.flat.next, b = blist->u.flat.next; ;
		a = a->next, b = b->next) {
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
	struct L_(listlink) *a = from->u.flat.next, *b, *temp;
	assert(from);
	if(!(b = a->next)) return;
	while(b->next) {
		if(PL_(compare)(a, b)) {
			a = b, b = b->next;
		} else {
			temp = b, b = b->next;
			L_(list_remove)(temp);
			if(to) L_(list_add_before)(&to->u.as_tail.tail, temp);
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
	PL_(boolean)(a, b, LIST_SUBTRACTION_AB | LIST_DEFAULT_A, result);
}

/** Moves the union of `a` and `b` as sequential sorted individual elements to
 `result`. Equal elements are moved from `a`. All parameters must be unique or
 can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:A, a:B, b:C, a:D)` would be moved to `result`.
 @order \O(|`a`| + |`b`|) @allow */
static void L_(list_union_to)(struct L_(list) *const a,
	struct L_(list) *const b, struct L_(list) *const result) {
	PL_(boolean)(a, b, LIST_SUBTRACTION_AB | LIST_SUBTRACTION_BA
		| LIST_INTERSECTION | LIST_DEFAULT_A | LIST_DEFAULT_B, result);
}

/** Moves the intersection of `a` and `b` as sequential sorted individual
 elements to `result`. Equal elements are moved from `a`. All parameters must
 be unique or can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:B)` would be moved to `result`.
 @order \O(|`a`| + |`b`|) @allow */
static void L_(list_intersection_to)(struct L_(list) *const a,
	struct L_(list) *const b, struct L_(list) *const result) {
	PL_(boolean)(a, b, LIST_INTERSECTION, result);
}

/** Moves `a` exclusive-or `b` as sequential sorted individual elements to
 `result`. Equal elements are moved from `a`. All parameters must be unique or
 can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:A, b:C, a:D)` would be moved to `result`.
 @order O(|`a`| + |`b`|) @allow */
static void L_(list_xor_to)(struct L_(list) *const a, struct L_(list) *const b,
	struct L_(list) *const result) {
	PL_(boolean)(a, b, LIST_SUBTRACTION_AB | LIST_SUBTRACTION_BA
		| LIST_DEFAULT_A | LIST_DEFAULT_B, result);
}

#endif /* comp --> */

/* <!-- iterate interface */

/** Contains all iteration parameters. (Since this is a permutation, the
 iteration is defined by none other then itself. Used for traits.) */
struct PL_(iterator) { struct L_(listlink) *node; };

/** Loads `list` into `it`. @implements begin */
static void PL_(begin)(struct PL_(iterator) *const it,
	const struct L_(list) *const list)
	{ assert(it && list), it->node = L_(list_head)(list); }

/** Advances `it`. @implements next */
static const struct L_(listlink) *PL_(next)(struct PL_(iterator) *const it) {
	return assert(it), it->node ? it->node = L_(list_next)(it->node): 0;
}

/* iterate --> */

/* <!-- box (multiple traits) */
#define BOX_ PL_
#define BOX_CONTAINER struct L_(list)
#define BOX_CONTENTS struct L_(listlink)

#ifdef LIST_TEST /* <!-- test */
/* Forward-declare. */
static void (*PL_(to_string))(const struct L_(listlink) *, char (*)[12]);
static const char *(*PL_(list_to_string))(const struct L_(list) *);
#include "../test/test_list.h"
#endif /* test --> */

static void PL_(unused_base_coda)(void);
static void PL_(unused_base)(void) {
	L_(list_head)(0); L_(list_tail)(0); L_(list_previous)(0); L_(list_next)(0);
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
	PL_(begin)(0, 0); PL_(next)(0); PL_(unused_base_coda)();
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


#else /* to string trait --><!-- compare trait */


#undef LIST_COMPARE


#endif /* traits --> */


#ifdef LIST_EXPECT_TRAIT /* <!-- trait */
#undef LIST_EXPECT_TRAIT
#else /* trait --><!-- !trait */
#ifdef LIST_TEST
#error No to LIST_TO_STRING traits defined for LIST_TEST.
#endif
#undef LIST_NAME
#undef BOX_
#undef BOX_CONTAINER
#undef BOX_CONTENTS
/* box (multiple traits) --> */
#endif /* !trait --> */
#undef LIST_TO_STRING_TRAIT
#undef LIST_COMPARE_TRAIT
#undef LIST_TRAITS
