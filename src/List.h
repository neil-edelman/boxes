/** @license 2017 Neil Edelman, distributed under the terms of the
 [MIT License](https://opensource.org/licenses/MIT).

 @subtitle Doubly-Linked List

 ![Example of a stochastic skip-list.](../web/list.png)

 In parlance of <Thareja 2014, Data Structures>, <tag:<N>list> is a circular
 header doubly-linked list of <tag:<N>list_node>. The header, or sentinel,
 resides in `<N>list`. This is a closed structure, such that with with a
 pointer to any element, it is possible to extract the entire list in
 \O(`size`). It only provides an order, and is not very useful without
 enclosing `<N>list_node` in another `struct`; this is useful for multi-linked
 elements.

 `<N>list` is not synchronised. Errors are returned with `errno`. The
 parameters are preprocessor macros, and are all undefined at the end of the
 file for convenience. Assertions are used in this file; to stop them, define
 `NDEBUG` before `assert.h`.

 @param[LIST_NAME]
 `<N>` that satisfies `C` naming conventions when mangled; required. `<PN>` is
 private, whose names are prefixed in a manner to avoid collisions.

 @param[LIST_COMPARE]
 Optional total-order function satisfying <typedef:<PN>compare_fn>.
 (fixme: move to trait.)

 @param[LIST_EXPECT_TRAIT]
 Do not un-define certain variables for subsequent inclusion in a trait.

 @param[LIST_TO_STRING_NAME, LIST_TO_STRING]
 To string trait contained in <to_string.h>; `<A>` that satisfies `C` naming
 conventions when mangled and function implementing <typedef:<PA>to_string_fn>.
 There can be multiple to string traits, but only one can omit
 `LIST_TO_STRING_NAME`.

 @param[LIST_TEST]
 To string trait contained in <../test/test_list.h>; optional unit testing
 framework using `assert`. Can only be defined once _per_ `Array`. Must be
 defined equal to a (random) filler function, satisfying
 <typedef:<PN>action_fn>. Output will be shown with the to string trait in
 which it's defined; provides tests for the base code and all later traits.

 @std C89
 @cf [array](https://github.com/neil-edelman/array)
 @cf [heap](https://github.com/neil-edelman/heap)
 @cf [orcish](https://github.com/neil-edelman/orcish)
 @cf [pool](https://github.com/neil-edelman/pool)
 @cf [set](https://github.com/neil-edelman/set)
 @cf [trie](https://github.com/neil-edelman/trie) */

#include <assert.h>


#ifndef LIST_NAME
#error Generic LIST_NAME undefined.
#endif
#if defined(LIST_TO_STRING_NAME) || defined(LIST_TO_STRING)
#define LIST_TO_STRING_TRAIT 1
#else
#define LIST_TO_STRING_TRAIT 0
#endif
#define LIST_TRAITS LIST_TO_STRING_TRAIT
#if LIST_TRAITS > 1
#error Only one trait per include is allowed; use LIST_EXPECT_TRAIT.
#endif
#if LIST_TRAITS != 0 && (!defined(N_) || !defined(CAT) || !defined(CAT_))
#error N_ or CAT_? not yet defined; use LIST_EXPECT_TRAIT?
#endif
#if (LIST_TRAITS == 0) && defined(LIST_TEST)
#error LIST_TEST must be defined in LIST_TO_STRING trait.
#endif
#if defined(LIST_TO_STRING_NAME) && !defined(LIST_TO_STRING)
#error LIST_TO_STRING_NAME requires LIST_TO_STRING.
#endif


#if LIST_TRAITS == 0 /* <!-- base code */


/* <Kernighan and Ritchie, 1988, p. 231>. */
#if defined(N_) || defined(PN_) \
	|| (defined(LIST_SUBTYPE) ^ (defined(CAT) || defined(CAT_)))
#error Unexpected P?N_ or P?CAT_?; possible stray LIST_EXPECT_TRAIT?
#endif
#ifndef LIST_SUBTYPE /* <!-- !sub-type */
#define CAT_(x, y) x ## _ ## y
#define CAT(x, y) CAT_(x, y)
#endif /* !sub-type --> */
#define N_(thing) CAT(LIST_NAME, thing)
#define PN_(thing) CAT(list, N_(thing))

/** Storage of this structure is the responsibility of the caller. One can only
 be in one list at a time; adding to another list while in a list destroys the
 integrity of the original list, see <fn:<N>list_remove>.

 ![States.](../web/node-states.png) */
struct N_(list_node);
struct N_(list_node) { struct N_(list_node) *prev, *next; };

/** Serves as head and tail for linked-list of <tag:<N>list_node>. Use
 <fn:<N>list_clear> to initialise the list. Because this list is closed; that
 is, given a valid pointer to an element, one can determine all others, null
 values are not allowed and it is _not_ the same as `{0}`.

 ![States.](../web/states.png) */
struct N_(list);
struct N_(list) {
	/* These are sentinels such that `head.prev` and `tail.next` are always and
	 the only ones to be null. */
	struct N_(list_node) head, tail;
};

/** Operates by side-effects on the node. */
typedef void (*PN_(action_fn))(struct N_(list_node) *);

/** Returns (Non-zero) true or (zero) false when given a node. */
typedef int (*PN_(predicate_fn))(const struct N_(list_node) *);

/** Returns less then, equal to, or greater then zero, inducing an ordering
 between `a` and `b`. */
typedef int (*PN_(compare_fn))(const struct N_(list_node) *a,
	const struct N_(list_node) *b);

/** Cats all `from` in front of `node`, (don't make `node` `head`); `from` will
 be empty after. Careful that `node` is not in `from` because that will just
 erase the list. @order \Theta(1) */
static void PN_(move)(struct N_(list) *const from,
	struct N_(list_node) *const node) {
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
static struct N_(list_node) *N_(list_first)(const struct N_(list) *const list) {
	struct N_(list_node) *link;
	assert(list);
	link = list->head.next, assert(link);
	return link->next ? link : 0;
}

/** @return A pointer to the last element of `list`, if it exists.
 @order \Theta(1) @allow */
static struct N_(list_node) *N_(list_last)(const struct N_(list) *const list) {
	struct N_(list_node) *link;
	assert(list);
	link = list->tail.prev, assert(link);
	return link->prev ? link : 0;
}

/** @return The previous element. When `link` is the first element, returns
 null. @order \Theta(1) @allow */
static struct N_(list_node) *N_(list_previous)(struct N_(list_node) *link) {
	assert(link && link->prev);
	link = link->prev;
	return link->prev ? link : 0;
}

/** @return The next element. When `link` is the last element, returns null.
 @order \Theta(1) @allow */
static struct N_(list_node) *N_(list_next)(struct N_(list_node) *link) {
	assert(link && link->next);
	link = link->next;
	return link->next ? link : 0;
}

/** Clears and initialises `list`.  @order \Theta(1) @allow */
static void N_(list_clear)(struct N_(list) *const list) {
	assert(list);
	list->head.prev = list->tail.next = 0;
	list->head.next = &list->tail;
	list->tail.prev = &list->head;
}

/** `add` before `anchor`. @order \Theta(1) @allow */
static void N_(list_add_before)(struct N_(list_node) *const anchor,
	struct N_(list_node) *const add) {
	assert(anchor && add && anchor != add && anchor->prev);
	add->prev = anchor->prev;
	add->next = anchor;
	anchor->prev->next = add;
	anchor->prev = add;
}

/** `add` after `anchor`. @order \Theta(1) @allow */
static void N_(list_add_after)(struct N_(list_node) *const anchor,
	struct N_(list_node) *const add) {
	assert(anchor && add && anchor != add && anchor->next);
	add->prev = anchor;
	add->next = anchor->next;
	anchor->next->prev = add;
	anchor->next = add;
}

/** Adds `add` to the beginning of `list`. @order \Theta(1) @allow */
static void N_(list_unshift)(struct N_(list) *const list,
	struct N_(list_node) *const add)
	{ assert(list && add), N_(list_add_after)(&list->head, add); }

/** Adds `add` to the end of `list`. @order \Theta(1) @allow */
static void N_(list_push)(struct N_(list) *const list,
	struct N_(list_node) *const add)
	{ assert(list && add), N_(list_add_before)(&list->tail, add); }

/** Remove `node`. @order \Theta(1) @allow */
static void N_(list_remove)(struct N_(list_node) *const node) {
	assert(node && node->prev && node->next);
	node->prev->next = node->next;
	node->next->prev = node->prev;
	node->prev = node->next = 0;
}

/** Removes the first element of `list` and returns it, if any.
 @order \Theta(1) @allow */
static struct N_(list_node) *N_(list_shift)(struct N_(list) *const list) {
	struct N_(list_node) *node;
	assert(list && list->head.next);
	if(!(node = list->head.next)->next) return 0;
	N_(list_remove)(node);
	return node;
}

/** Removes the last element of `list` and returns it, if any.
 @order \Theta(1) @allow */
static struct N_(list_node) *N_(list_pop)(struct N_(list) *const list) {
	struct N_(list_node) *node;
	assert(list && list->tail.prev);
	if(!(node = list->tail.prev)->prev) return 0;
	N_(list_remove)(node);
	return node;
}

/** Moves the elements `from` onto `to` at the end.
 @param[to] If null, then it removes elements from `from`.
 @order \Theta(1) @allow */
static void N_(list_to)(struct N_(list) *const from,
	struct N_(list) *const to) {
	assert(from && from != to);
	if(!to) { N_(list_clear)(from); return; }
	PN_(move)(from, &to->tail);
}

/** Moves the elements `from` immediately before `anchor`.
 @order \Theta(1) @allow */
static void N_(list_to_before)(struct N_(list) *const from,
	struct N_(list_node) *const anchor) {
	assert(from && anchor);
	PN_(move)(from, anchor);
}

/** Moves all elements `from` onto `to` at the end if `predicate` is true.
 @param[to] If null, then it removes elements.
 @order \Theta(|`from`|) \times \O(`predicate`) @allow */
static void N_(list_to_if)(struct N_(list) *const from,
	struct N_(list) *const to, const PN_(predicate_fn) predicate) {
	struct N_(list_node) *link, *next_link;
	assert(from && from != to && predicate);
	for(link = from->head.next; (next_link = link->next); link = next_link) {
		if(!predicate(link)) continue;
		N_(list_remove)(link);
		if(to) N_(list_add_before)(&to->tail, link);
	}
}

/** Performs `action` for each element in `list` in order.
 @param[action] Can be to delete the element.
 @order \Theta(|`list`|) \times O(`action`) @allow */
static void N_(list_for_each)(struct N_(list) *const list,
	const PN_(action_fn) action) {
	struct N_(list_node) *x, *next_x;
	assert(list && action);
	for(x = list->head.next; (next_x = x->next); x = next_x)
		action(x);
}

/** Iterates through `list` and calls `predicate` until it returns true.
 @return The first `predicate` that returned true, or, if the statement is
 false on all, null.
 @order \O(|`list`|) \times \O(`predicate`) @allow */
static struct N_(list_node) *N_(list_any)(const struct N_(list) *const list,
	const PN_(predicate_fn) predicate) {
	struct N_(list_node) *link, *next_link;
	assert(list && predicate);
	for(link = list->head.next; (next_link = link->next); link = next_link)
		if(predicate(link)) return link;
	return 0;
}

/** Corrects `list` ends to compensate for memory relocation of the list
 itself. @order \Theta(1) @allow */
static void N_(list_self_correct)(struct N_(list) *const list) {
	assert(list);
	if(list->head.next == list->tail.prev + 1) {
		list->head.next = &list->tail;
		list->tail.prev = &list->head;
	} else {
		list->head.next->prev = &list->head;
		list->tail.prev->next = &list->tail;
	}
}

#ifdef LIST_COMPARE /* <!-- comp */

/* Constants across multiple includes in the same translation unit. */
#ifndef LIST_H /* <!-- h */
#define LIST_H
/* <fn:<PN>boolean> operations bit-vector; dummy `LO_` ensures closed. */
enum ListOperation {
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
#endif /* h --> */

/* Check that `LIST_COMPARE` is a function implementing
 <typedef:<PN>compare_fn>. */
static const PN_(compare_fn) PN_(compare) = (LIST_COMPARE);

/** Private: `alist` `mask` `blist` -> `result`. Prefers `a` to `b` when equal.
 @order \O(|`a`| + |`b`|) */
static void PN_(boolean)(struct N_(list) *const alist,
	struct N_(list) *const blist, const enum ListOperation mask,
	struct N_(list) *const result) {
	struct N_(list_node) *a = alist ? alist->head.next : 0,
		*b = blist ? blist->head.next : 0, *temp;
	int comp;
	assert((!result || (result != alist && result != blist))
		&& (!alist || (alist != blist)));
	if(a && b) {
		while(a->next && b->next) {
			comp = PN_(compare)(a, b);
			if(comp < 0) {
				temp = a, a = a->next;
				if(mask & LO_SUBTRACTION_AB) {
					N_(list_remove)(temp);
					if(result) N_(list_add_before)(&result->tail, temp);
				}
			} else if(comp > 0) {
				temp = b, b = b->next;
				if(mask & LO_SUBTRACTION_BA) {
					N_(list_remove)(temp);
					if(result) N_(list_add_before)(&result->tail, temp);
				}
			} else {
				temp = a, a = a->next, b = b->next;
				if(mask & LO_INTERSECTION) {
					N_(list_remove)(temp);
					if(result) N_(list_add_before)(&result->tail, temp);
				}
			}
		}
	}
	if(a && mask & LO_DEFAULT_A) {
		while((temp = a, a = a->next)) {
			N_(list_remove)(temp);
			if(result) N_(list_add_before)(&result->tail, temp);
		}
	}
	if(b && mask & LO_DEFAULT_B) {
		while((temp = b, b = b->next)) {
			N_(list_remove)(temp);
			if(result) N_(list_add_before)(&result->tail, temp);
		}
	}
}

/* A run is a sequence of values in the array that is weakly increasing. */
struct PN_(Run) { struct N_(list_node) *head, *tail; size_t size; };
/* Store the maximum capacity for the indexing with {size_t}. (Much more then
 we need, in most cases.) \${
 \> range(runs) = Sum_{k=0}^runs 2^{runs-k} - 1
 \>             = 2^{runs+1} - 2
 \> 2^bits      = 2 (r^runs - 1)
 \> runs        = log(2^{bits-1} + 1) / log 2
 \> runs       <= 2^{bits - 1}, 2^{bits + 1} > 0} */
struct PN_(Runs) {
	struct PN_(Run) run[(sizeof(size_t) << 3) - 1];
	size_t run_no;
};

/** Inserts the first element from the larger of two sorted `r`, then merges
 the rest. */
static void PN_(merge_runs)(struct PN_(Runs) *const r) {
	struct PN_(Run) *const run_a = r->run + r->run_no - 2;
	struct PN_(Run) *const run_b = run_a + 1;
	struct N_(list_node) *a = run_a->tail, *b = run_b->head, *chosen;
	assert(r->run_no >= 2);
	/* In the absence of any real information, assume that the elements farther
	 in the list are generally more apt to be at the back, _viz_, adaptive. */
	if(run_a->size <= run_b->size) {
		struct N_(list_node) *prev_chosen;
		/* Run `a` is smaller: downwards insert `b.head` followed by upwards
		 merge. Insert the first element of `b` downwards into `a`. */
		for( ; ; ) {
			if(PN_(compare)(a, b) <= 0) { chosen = a; a = a->next; break; }
			if(!a->prev) { run_a->head = run_b->head; chosen = b; b = b->next;
				break; }
			a = a->prev;
		}
		/* Merge upwards; while the lists are interleaved. */
		while(chosen->next) {
			prev_chosen = chosen;
			if(PN_(compare)(a, b) > 0) chosen = b, b = b->next;
			else chosen = a, a = a->next;
			prev_chosen->next = chosen;
			chosen->prev = prev_chosen;
		}
		/* Splice the one list left. */
		if(!a) b->prev = chosen, chosen->next = b, run_a->tail = run_b->tail;
		else a->prev = chosen, chosen->next = a;
	} else {
		struct N_(list_node) *next_chosen;
		int is_a_tail = 0;
		/* Run `b` is smaller; upwards insert followed by downwards merge.
		 Insert the last element of `a` upwards into `b`. */
		for( ; ; ) {
			if(PN_(compare)(a, b) <= 0) { chosen = b; b = b->prev; break; }
			/* Here, `a > b`. */
			if(!b->next) { is_a_tail = -1; chosen = a; a = a->prev; break; }
			b = b->next;
		}
		if(!is_a_tail) run_a->tail = run_b->tail;
		/* Merge downwards, while the lists are interleaved. */
		while(chosen->prev) {
			next_chosen = chosen;
			if(PN_(compare)(a, b) > 0) chosen = a, a = a->prev;
			else chosen = b, b = b->prev;
			next_chosen->prev = chosen;
			chosen->next = next_chosen;
		}
		/* Splice the one list left. */
		if(!a) b->next = chosen, chosen->prev = b, run_a->head = run_b->head;
		else a->next = chosen, chosen->prev = a;
	}
	run_a->size += run_b->size;
	r->run_no--;
}

/** Natural merge sorts `list`. */
static void PN_(natural)(struct N_(list) *const list) {
	/* fixme: This is half-a-KB; use recursion properly. */
	struct PN_(Runs) runs;
	struct PN_(Run) *new_run;
	/* Part of the state machine for classifying points. */
	enum { UNSURE, INCREASING, DECREASING } mono;
	/* The data that we are sorting. */
	struct N_(list_node) *a, *b, *c, *first_iso_a;
	/* `run_count` is different from `runs.run_no` in that it only increases;
	 only used for calculating the path up the tree. */
	size_t run_count, rc;
	/* The value of the comparison. */
	int comp;
	assert(list);
	/* Needs an element. */
	a = list->head.next, assert(a);
	if(!(b = a->next)) return;
	/* Reset the state machine and output to just `a` in the first run. */
	mono = UNSURE;
	runs.run_no = 1;
	new_run = runs.run + 0, run_count = (size_t)1;
	new_run->size = 1;
	first_iso_a = new_run->head = new_run->tail = a;
	/* While `a` and `b` are elements, (that are consecutive.) */
	for(c = b->next; c; a = b, b = c, c = c->next) {
		comp = PN_(compare)(a, b);
		/* State machine that considers runs in both directions -- in practice,
		 slightly slower than only considering increasing runs on most cases. */
		if(comp < 0) { /* `a < b`, increasing -- good. */
			if(mono != DECREASING) { /* If decreasing, inflection. */
				mono = INCREASING;
				new_run->size++;
				continue;
			}
		} else if(comp > 0) { /* Decreasing; reverse preserving stability. */
			if(mono != INCREASING) { /* If increasing, inflection. */
				mono = DECREASING;
				b->next = first_iso_a;
				first_iso_a->prev = b;
				new_run->head = first_iso_a = b;
				new_run->size++;
				continue;
			}
			new_run->tail = a; /* Terminating an increasing sequence. */
		} else { /* `a == b`. */
			if(mono == DECREASING) { /* Extend. */
				struct N_(list_node) *const a_next = a->next;
				b->next = a_next;
				a_next->prev = b;
				a->next = b;
				b->prev = a;
			} else { /* Monotone or weakly increasing. */
				new_run->tail = b;
			}
			new_run->size++;
			continue;
		}
		/* Head and tail don't necessarily correspond to the first and last. */
		new_run->head->prev = new_run->tail->next = 0;
		/* Greedy merge: keeps space to `O(log n)` instead of `O(n)`. */
		for(rc = run_count; !(rc & 1) && runs.run_no >= 2; rc >>= 1)
			PN_(merge_runs)(&runs);
		/* Reset the state machine and output to just `b` at the next run. */
		mono = UNSURE;
		assert(runs.run_no < sizeof(runs.run) / sizeof(*runs.run));
		new_run = runs.run + runs.run_no++, run_count++;
		new_run->size = 1;
		new_run->head = new_run->tail = first_iso_a = b;
	}
	/* Terminating the last increasing sequence. */
	if(mono == INCREASING) new_run->tail = a;
	new_run->tail->next = new_run->head->prev = 0;
	/* Clean up the rest; when only one, propagate `list_runs[0]` to head. */
	while(runs.run_no > 1) PN_(merge_runs)(&runs);
	runs.run[0].head->prev = &list->head;
	runs.run[0].tail->next = &list->tail;
	list->head.next = runs.run[0].head;
	list->tail.prev = runs.run[0].tail;
}

/** Performs a stable, adaptive sort of `list` according to `compare`. Requires
 `LIST_COMPARE`. <Peters 2002, Timsort>, _via_ <McIlroy 1993, Optimistic>, does
 long merges by galloping, but we don't have random access to the data because
 we are in a linked-list; this does natural merge sort.
 @order \Omega(|`list`|), \O(|`list`| log |`list`|) @allow */
static void N_(list_sort)(struct N_(list) *const list)
	{ assert(list), PN_(natural)(list); }

/** Merges from `from` into `to`. If the elements are sorted in both lists,
 then the elements of `list` will be sorted.
 @order \O(|`from`| + |`to`|). */
static void N_(list_merge)(struct N_(list) *const from,
	struct N_(list) *const to) {
	struct N_(list_node) *cur, *a, *b;
	assert(from && from->head.next && to && to->head.next && from != to);
	/* `blist` empty -- that was easy. */
	if(!(b = from->head.next)->next) return;
	/* `alist` empty -- `O(1)` <fn:<PN>move> is more efficient. */
	if(!(a = to->head.next)->next)
	{ PN_(move)(from, &to->tail); return; }
	/* Merge */
	for(cur = &to->head; ; ) {
		if(PN_(compare)(a, b) < 0) {
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
 considered equal. @implements <typedef:<PN>compare_fn>
 @order \Theta(min(|`alist`|, |`blist`|)) @allow */
static int N_(list_compare)(const struct N_(list) *const alist,
	const struct N_(list) *const blist) {
	struct N_(list_node) *a, *b;
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
		} else if((diff = PN_(compare)(a, b))) {
			return diff;
		}
	}
}

/** Moves all local-duplicates of `from` to the end of `to`.

 For example, if `from` is `(A, B, B, A)`, it would concatenate `(B)` to `to`
 and leave `(A, B, A)` in `from`. If one <fn:<N>list_sort> `from` first,
 `(A, A, B, B)`, the global duplicates will be transferred, `(A, B)`.
 @order \O(|`from`|) @allow */
static void N_(list_duplicates_to)(struct N_(list) *const from,
	struct N_(list) *const to) {
	struct N_(list_node) *a = from->head.next, *b, *temp;
	assert(from);
	if(!(b = a->next)) return;
	while(b->next) {
		if(PN_(compare)(a, b)) {
			a = b, b = b->next;
		} else {
			temp = b, b = b->next;
			N_(list_remove)(temp);
			if(to) N_(list_add_before)(&to->tail, temp);
		}
	}
}

/** Subtracts `a` from `b`, as sequential sorted individual elements, and moves
 it to `result`. All elements are removed from `a`. All parameters must be
 unique or can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:A, a:D)` would be moved to `result`.
 @order \O(|`a`| + |`b`|) @allow */
static void N_(list_subtraction_to)(struct N_(list) *const a,
	struct N_(list) *const b, struct N_(list) *const result) {
	PN_(boolean)(a, b, LO_SUBTRACTION_AB | LO_DEFAULT_A, result);
}

/** Moves the union of `a` and `b` as sequential sorted individual elements to
 `result`. Equal elements are moved from `a`. All parameters must be unique or
 can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:A, a:B, b:C, a:D)` would be moved to `result`.
 @order \O(|`a`| + |`b`|) @allow */
static void N_(list_union_to)(struct N_(list) *const a,
	struct N_(list) *const b, struct N_(list) *const result) {
	PN_(boolean)(a, b, LO_SUBTRACTION_AB | LO_SUBTRACTION_BA | LO_INTERSECTION
		| LO_DEFAULT_A | LO_DEFAULT_B, result);
}

/** Moves the intersection of `a` and `b` as sequential sorted individual
 elements to `result`. Equal elements are moved from `a`. All parameters must
 be unique or can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:B)` would be moved to `result`.
 @order \O(|`a`| + |`b`|) @allow */
static void N_(list_intersection_to)(struct N_(list) *const a,
	struct N_(list) *const b, struct N_(list) *const result) {
	PN_(boolean)(a, b, LO_INTERSECTION, result);
}

/** Moves `a` exclusive-or `b` as sequential sorted individual elements to
 `result`. Equal elements are moved from `a`. All parameters must be unique or
 can be null.

 For example, if `a` contains `(A, B, D)` and `b` contains `(B, C)` then
 `(a:A, b:C, a:D)` would be moved to `result`.
 @order O(|`a`| + |`b`|) @allow */
static void N_(list_xor_to)(struct N_(list) *const a, struct N_(list) *const b,
	struct N_(list) *const result) {
	PN_(boolean)(a, b, LO_SUBTRACTION_AB | LO_SUBTRACTION_BA | LO_DEFAULT_A
		| LO_DEFAULT_B, result);
}

#endif /* comp --> */

#if defined(ITERATE) || defined(ITERATE_BOX) || defined(ITERATE_TYPE) \
	|| defined(ITERATE_BEGIN) || defined(ITERATE_NEXT)
#error Unexpected ITERATE*.
#endif

/** Contains all iteration parameters. */
struct PN_(iterator);
struct PN_(iterator) { struct N_(list_node) *node; };

/** Loads `list` into `it`. @implements begin */
static void PN_(begin)(struct PN_(iterator) *const it,
	const struct N_(list) *const list)
	{ assert(it && list), it->node = list->head.next; }

/** Advances `it`. @implements next */
static const struct N_(list_node) *PN_(next)(struct PN_(iterator) *const it) {
	struct N_(list_node) *n;
	assert(it && it->node);
	return (it->node = (n = it->node)->next) ? n : 0;
}

#define ITERATE struct PN_(iterator)
#define ITERATE_BOX struct N_(list)
#define ITERATE_TYPE struct N_(list_node)
#define ITERATE_BEGIN PN_(begin)
#define ITERATE_NEXT PN_(next)

static void PN_(unused_base_coda)(void);
static void PN_(unused_base)(void) {
	N_(list_first)(0); N_(list_last)(0); N_(list_previous)(0); N_(list_next)(0);
	N_(list_clear)(0); N_(list_add_before)(0, 0); N_(list_add_after)(0, 0);
	N_(list_unshift)(0, 0); N_(list_push)(0, 0); N_(list_remove)(0);
	N_(list_shift)(0); N_(list_pop)(0); N_(list_to)(0, 0);
	N_(list_to_before)(0, 0); N_(list_to_if)(0, 0, 0); N_(list_for_each)(0, 0);
	N_(list_any)(0, 0); N_(list_self_correct)(0);
#ifdef LIST_COMPARE /* <!-- comp */
	N_(list_sort)(0); N_(list_merge)(0, 0); N_(list_compare)(0, 0);
	N_(list_duplicates_to)(0, 0); N_(list_subtraction_to)(0, 0, 0);
	N_(list_union_to)(0, 0, 0); N_(list_intersection_to)(0, 0, 0);
	N_(list_xor_to)(0, 0, 0);
#endif /* comp --> */
	PN_(begin)(0, 0); PN_(next)(0); PN_(unused_base_coda)();
}
static void PN_(unused_base_coda)(void) { PN_(unused_base)(); }


#elif defined(LIST_TO_STRING) /* base code --><!-- to string trait */


#ifdef LIST_TO_STRING_NAME /* <!-- name */
#define A_(thing) CAT(N_(list), CAT(LIST_TO_STRING_NAME, thing))
#else /* name --><!-- !name */
#define A_(thing) CAT(N_(list), thing)
#endif /* !name --> */
#define TO_STRING LIST_TO_STRING
#include "to_string.h" /** \include */

#if !defined(LIST_TEST_BASE) && defined(LIST_TEST) /* <!-- test */
#define LIST_TEST_BASE /* Only one instance of base tests. */
#include "../test/test_list.h" /** \include */
#endif /* test --> */

#undef A_
#undef LIST_TO_STRING
#ifdef LIST_TO_STRING_NAME
#undef LIST_TO_STRING_NAME
#endif


#endif /* traits --> */


#ifdef LIST_EXPECT_TRAIT /* <!-- trait */
#undef LIST_EXPECT_TRAIT
#else /* trait --><!-- !trait */
#ifndef LIST_SUBTYPE /* <!-- !sub-type */
#undef CAT
#undef CAT_
#else /* !sub-type --><!-- sub-type */
#undef LIST_SUBTYPE
#endif /* sub-type --> */
#undef N_
#undef PN_
#undef LIST_NAME
#ifdef LIST_COMPARE
#undef LIST_COMPARE
#endif
#ifdef LIST_TEST
#undef LIST_TEST
#endif
#ifdef LIST_TEST_BASE
#undef LIST_TEST_BASE
#endif
#undef ITERATE
#undef ITERATE_BOX
#undef ITERATE_TYPE
#undef ITERATE_BEGIN
#undef ITERATE_NEXT
#endif /* !trait --> */

#undef LIST_TO_STRING_TRAIT
#undef LIST_TRAITS
